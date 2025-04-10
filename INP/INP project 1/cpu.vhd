-- cpu.vhd: Simple 8-bit CPU (BrainFuck interpreter)
-- Copyright (C) 2024 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): jmeno <login AT stud.fit.vutbr.cz>
--
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
 port (
   CLK   : in std_logic;  -- hodinovy signal
   RESET : in std_logic;  -- asynchronni reset procesoru
   EN    : in std_logic;  -- povoleni cinnosti procesoru
 
   -- synchronni pamet RAM
   DATA_ADDR  : out std_logic_vector(12 downto 0); -- adresa do pameti
   DATA_WDATA : out std_logic_vector(7 downto 0); -- mem[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
   DATA_RDATA : in std_logic_vector(7 downto 0);  -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
   DATA_RDWR  : out std_logic;                    -- cteni (1) / zapis (0)
   DATA_EN    : out std_logic;                    -- povoleni cinnosti
   
   -- vstupni port
   IN_DATA   : in std_logic_vector(7 downto 0);   -- IN_DATA <- stav klavesnice pokud IN_VLD='1' a IN_REQ='1'
   IN_VLD    : in std_logic;                      -- data platna
   IN_REQ    : out std_logic;                     -- pozadavek na vstup data
   
   -- vystupni port
   OUT_DATA : out  std_logic_vector(7 downto 0);  -- zapisovana data
   OUT_BUSY : in std_logic;                       -- LCD je zaneprazdnen (1), nelze zapisovat
   OUT_INV  : out std_logic;                      -- pozadavek na aktivaci inverzniho zobrazeni (1)
   OUT_WE   : out std_logic;                      -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'

   -- stavove signaly
   READY    : out std_logic;                      -- hodnota 1 znamena, ze byl procesor inicializovan a zacina vykonavat program
   DONE     : out std_logic                       -- hodnota 1 znamena, ze procesor ukoncil vykonavani programu (narazil na instrukci halt)
 );
end cpu;


-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of cpu is

 -- pri tvorbe kodu reflektujte rady ze cviceni INP, zejmena mejte na pameti, ze 
 --   - nelze z vice procesu ovladat stejny signal,
 --   - je vhodne mit jeden proces pro popis jedne hardwarove komponenty, protoze pak
 --      - u synchronnich komponent obsahuje sensitivity list pouze CLK a RESET a 
 --      - u kombinacnich komponent obsahuje sensitivity list vsechny ctene signaly. 

--------------------- Řídící Signály ---------------------
-- Program Counter
signal pc_reg : std_logic_vector(12 downto 0) := (others => '0');
signal pc_inc : std_logic;
signal pc_dec : std_logic;

-- Memory Pointer
signal ptr_reg : std_logic_vector(12 downto 0) := (others => '0');
signal ptr_inc : std_logic;
signal ptr_dec : std_logic;

-- Temporary register
signal tmp_reg : std_logic_vector(7 downto 0) := (others => '0');
signal tmp_ld : std_logic;  -- Nacteni hodnoty z pameti do pomocneho registru
signal tmp_st : std_logic;

-- Accumulator register
signal acc_reg : std_logic_vector(7 downto 0) := (others => '0');
signal acc_inc : std_logic;
signal acc_dec  : std_logic;

-- Multiplexerer selectors
signal mx1_selector : std_logic := '0';
signal mx2_selector : std_logic_vector(1 downto 0) := (others => '0');

---------- Deterministický konečný automat (DKA) ----------
-- Množina stavů pro konečný automat
type fsm_state is (
    sRESET, sPTR_INIT0, sPTR_INIT1, sIDLE, sFETCH, sDECODE, sOUT_WAIT, sIN_WAIT,
    sPTR_INC,     -- '>'
    sPTR_DEC,     -- '<'
    sCURRENT_INC, -- '+' 
    sCURRENT_DEC, -- '-'
    sLOOP_ENTRY0, sLOOP_ENTRY1, sLOOP_ENTRY2, sLOOP_ENTRY3,  -- '['
    sLOOP_EXIT0, sLOOP_EXIT1, sLOOP_EXIT2, sLOOP_EXIT3,   -- ']'
    sAUX_STORE,   -- '$'
    sAUX_LOAD,    -- '!'
    sOUT_CURRENT, -- '.'
    sIN_CURRENT,  -- ','
    sHALT   -- '@'
);

-- FSM driver
signal pstate : fsm_state;
signal nstate : fsm_state;

begin

fsm_pstate: process(RESET, CLK)
begin
  if (RESET = '1') then
    pstate <= sRESET;
  elsif (CLK'event) and (CLK='1') then
    if (EN = '1') then
      pstate <= nstate;
    end if;
  end if;
end process;

-- {FSM
fsm_nstate: process(pstate, DATA_RDATA)
begin 
  -- {INIT
  -- RAM + I/O
  DATA_RDWR <= '0';
  DATA_EN <= '0';
  IN_REQ <= '0';
  OUT_INV <= '0';
  OUT_WE <= '0';
  -- Program Counter
  pc_inc <= '0';
  pc_dec <= '0';
  -- Memory pointer
  ptr_inc <= '0';
  ptr_dec <= '0';
  -- Tmp register
  tmp_ld <= '0';
  tmp_st <= '0';
  -- Acc register
  acc_inc <= '0';
  acc_dec <= '0';
  -- INIT}
  case pstate is

    when sRESET =>
      READY <= '0';
      DONE <= '0';
      nstate <= sPTR_INIT0;
    
    when sPTR_INIT0 =>
      DATA_EN <= '1';
      DATA_RDWR <= '1';
      mx1_selector <= '1'; -- data memory mem[ptr]
      nstate <= sPTR_INIT1;

    when sPTR_INIT1 =>
      ptr_inc <= '1';
      if DATA_RDATA = X"40" then
        nstate <=  sIDLE;
      else nstate <= sPTR_INIT0;
      end if;

    when sIDLE =>
      READY <= '1'; -- Data ptr initialised, ready to operate
      nstate <= sFETCH;

    when sFETCH =>
      mx1_selector <= '0'; -- mem[pc]
      DATA_EN <= '1';
      DATA_RDWR <= '1';
      nstate <= sDECODE;

    when sDECODE =>
      pc_inc <= '1'; --instruction read
      case DATA_RDATA is
        when X"3E" => -- '>' ++ptr
          ptr_inc <= '1';
          nstate <= sFETCH;
        when X"3C" => -- '<' --ptr
          ptr_dec <= '1';
          nstate <= sFETCH;
        when X"2B" => -- '+' ++*ptr
          mx1_selector <= '1'; -- mem[ptr]
          DATA_EN <= '1';
          DATA_RDWR <= '1';
          nstate <= sCURRENT_INC;
        when X"2D" => -- '-' --*ptr
          mx1_selector <= '1';
          DATA_EN <= '1';
          DATA_RDWR <= '1';
          nstate <= sCURRENT_DEC;
        when X"5B" => -- '['
          nstate <= sLOOP_ENTRY0;
        when X"5D" => -- ']'
          nstate <= sLOOP_EXIT0;
        when X"24" => -- '$' *tmp = *ptr
          DATA_EN <= '1';
          DATA_RDWR <= '1';
          mx1_selector <= '1'; -- mem[ptr]
          nstate <= sAUX_STORE;
        when X"21" => -- '!' *ptr = *tmp
          nstate <= sAUX_LOAD;
        when X"2E" => -- '.' putchar 
          nstate <= sOUT_WAIT;
        when X"2C" => -- ',' getchar
          nstate <= sIN_WAIT;
        when X"40" => -- '@' konec
          nstate <= sHALT;
        when others =>
          nstate <= sFETCH; -- přeskočení znaků, které nejsou součástí instrukční sady
      end case;

    -- Aritmetické operace ++, --
    when sCURRENT_INC =>
      mx2_selector <= "11";
      DATA_EN <= '1';
      DATA_RDWR <= '0';
      nstate <= sFETCH;

    when sCURRENT_DEC =>
      mx2_selector <= "10";
      DATA_EN <= '1';
      DATA_RDWR <= '0';
      nstate <= sFETCH;

    -- Ulozeni do TMP registru
    when sAUX_STORE =>
      tmp_ld <= '1';
      nstate <= sFETCH;

    when sAUX_LOAD =>
      mx1_selector <= '1';
      mx2_selector <= "01";
      DATA_EN <= '1';
      DATA_RDWR <= '0';
      nstate <= sFETCH; 

    -- Vypis znaku na OUT
    when sOUT_WAIT =>
      mx1_selector <= '1';
      DATA_EN <= '1';
      DATA_RDWR <= '1';
      nstate <= sOUT_CURRENT;  

    when sOUT_CURRENT =>
      if(OUT_BUSY = '0') then
        OUT_WE <= '1';
        OUT_DATA <= DATA_RDATA;
        nstate <= sFETCH;
        OUT_INV <= '0';
      else 
        nstate <= sOUT_WAIT;
      end if;

    -- Nacteni znaku do pameti
    when sIN_WAIT =>
      IN_REQ <= '1';
      mx1_selector <= '1';
      DATA_EN <= '1';
      DATA_RDWR <= '0';
      nstate <= sIN_CURRENT;

    when sIN_CURRENT =>
      IN_REQ <= '1';
      if(IN_VLD = '1') then
        mx2_selector <= "00";
        nstate <= sFETCH;
      else 
        nstate <= sIN_WAIT;
      end if;

    -- Cyklus bez vnoreni
    when sLOOP_ENTRY0 =>
      mx1_selector <= '1'; -- mem[ptr]
      DATA_EN <= '1';
      DATA_RDWR <= '1';
      nstate <= sLOOP_ENTRY1;

    when sLOOP_ENTRY1 =>
      if(DATA_RDATA = "00000000") then -- *ptr == 0
        nstate <= sLOOP_ENTRY2;
      else nstate <= sFETCH;
      end if;

    when sLOOP_ENTRY2 =>
      mx1_selector <= '0'; -- mem[pc]
      DATA_EN <= '1';
      DATA_RDWR <= '1'; 
      nstate <= sLOOP_ENTRY3;

    when sLOOP_ENTRY3 =>
      if(DATA_RDATA = X"5D") then -- DATA_RDATA = ]
         nstate <= sFETCH;
      else pc_inc <= '1';
           nstate <= sLOOP_ENTRY2;
      end if;

    when sLOOP_EXIT0 =>
      mx1_selector <= '1'; -- mem[ptr]
      DATA_EN <= '1';
      DATA_RDWR <= '1';
      nstate <= sLOOP_EXIT1;

    when sLOOP_EXIT1 =>
      if(DATA_RDATA = "00000000") then
        nstate <= sFETCH;
      else nstate <= sLOOP_EXIT2;
      end if;

    when sLOOP_EXIT2 =>
      mx1_selector <= '0'; -- mem[pc]
      DATA_EN <= '1';
      DATA_RDWR <= '1'; 
      nstate <= sLOOP_EXIT3;

    when sLOOP_EXIT3 =>
      if(DATA_RDATA = x"5B") then -- DATA_RDATA = [
        nstate <= sLOOP_ENTRY0;
      else nstate <= sLOOP_EXIT2;
           pc_dec <= '1'; 
      end if;

    when sHALT =>
      DONE <= '1';
      nstate <= sHALT;

    when others => 
      nstate <= sHALT; -- Neznamy stav
   end case;

end process;
-- FSM}

---------------------- Multiplexery ----------------------
-- MX1 dataflow, přepínání mezi prog. čítačem a ukazatelem do paměti
mx1: with mx1_selector select
        DATA_ADDR <= pc_reg when '0',
                     ptr_reg when '1',
                     "0000000000000" when others;

-- MX2 dataflow, přepínání mezi vstupními daty, akumulátorem, data_rdata
mx2: with mx2_selector select
        DATA_WDATA <= IN_DATA when "00",
                      tmp_reg when "01",
                      DATA_RDATA - 1 when "10",
                      DATA_RDATA + 1 when "11",
                      (others => '0') when others;           
----------------------- Komponenty -----------------------

-- {Programovy citac
pc: process (RESET, CLK)
begin
  if (RESET = '1') then
    pc_reg <= (others => '0');
  elsif (CLK'event) and (CLK='1') then
    if (pc_inc = '1') then
      pc_reg <= pc_reg + 1;
    elsif (pc_dec = '1') then
      pc_reg <= pc_reg - 1;
    end if;
  end if;
end process;
-- Programovy citac}

-- {Ukazatel do pameti
ptr: process (RESET, CLK)
begin
  if (RESET = '1') then
    ptr_reg <= (others => '0');
  elsif (CLK'event) and (CLK = '1') then
    if (ptr_inc = '1') then
      ptr_reg <= ptr_reg + 1;
    elsif (ptr_dec ='1') then
      ptr_reg <= ptr_reg - 1;
    end if;
  end if;
end process;
-- Ukazatel do pameti}

-- {Tmp (Pomocny registr) 
tmp: process (RESET, CLK)
  begin
    if (RESET = '1') then
      tmp_reg <= (others => '0');
    elsif (CLK'event) and (CLK = '1') then  
      if(tmp_ld = '1') then
        tmp_reg <= DATA_RDATA;
      end if;
    end if;
end process;
-- Tmp}

-- {Akumulator (Cykly)
acc: process (RESET, CLK)
begin
  if (RESET = '1') then
    acc_reg <= (others => '0');
  elsif (CLK'event) and (CLK='1') then
    if (acc_inc = '1') then
      acc_reg <= acc_reg + 1;
    elsif (acc_dec = '1') then
      acc_reg <= acc_reg - 1;
    end if;
  end if;
end process;
-- Akumulator}

end behavioral;
