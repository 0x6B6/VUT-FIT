-- uart_rx.vhd: UART controller - receiving (RX) side
-- Author(s): Marek Pazúr (xpazurm00)

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;




-- Entity declaration (DO NOT ALTER THIS PART!)
entity UART_RX is
    port(
        CLK      : in std_logic;
        RST      : in std_logic;
        DIN      : in std_logic;
        DOUT     : out std_logic_vector(7 downto 0);
        DOUT_VLD : out std_logic
    );
end entity;



-- Architecture implementation (INSERT YOUR IMPLEMENTATION HERE)
architecture behavioral of UART_RX is
    -- Declaration of architecture signals
    signal bit_counter : std_logic_vector(3 downto 0); -- Counter of transferred bits

    signal clk_counter : std_logic_vector(4 downto 0); -- Counter of clock cycles
    signal clk_toggle : std_logic;  -- Clock enable (CE)
    
    signal data_transfer : std_logic; -- Toggle data transfer
    signal data_validate : std_logic; -- Toggle validation of transferred data
begin
    -- Instance of RX FSM, map signal interface
    fsm: entity work.UART_RX_FSM
    port map (
        CLK => CLK,
        RST => RST,
        DIN => DIN,

        BIT_COUNTER => bit_counter,
        CLK_COUNTER => clk_counter,
       
        CLK_TOGGLE => clk_toggle,
        DATA_TRANSFER => data_transfer,
        DATA_VALIDATE => data_validate
    );

    -- Data transfer process sensitive to CLK signal
    p_transfer: process(CLK)
    begin
        -- Asynchronous reset -> set counters, DOUT registers and data validation to zero
        if (RST = '1') then
            bit_counter <= (others => '0');
            clk_counter <= "00001";
            DOUT <= (others => '0');
            DOUT_VLD <= '0';

        elsif CLK'event and CLK = '1' then
            -- Set data validation to zero
            DOUT_VLD <= '0';

            -- If clk_toggle is enabled, count clock cycles, else set the counter to 00001
            if clk_toggle = '1' then
                clk_counter <= clk_counter + 1;
            else
                clk_counter <= "00001"; 
            end if;

            -- Validate data after transferring 8 bits with STOP BIT 
            if bit_counter = "1000" and data_validate = '1' then
                DOUT_VLD <= '1'; -- Validate transferred data
                bit_counter <= (others => '0'); -- Reset bit counter
            end if;

            -- If data is being transferred and 16+ clock cycles have taken place (MID BIT sample) then
            if data_transfer = '1' and clk_counter >= "10000" then
                -- Set the clock counter to 00001
                 clk_counter <= "00001";

                -- Assign DIN bit to corresponding register (DOUT LSB 0-7 MSB) by using demultiplexer (bit counter represents selector)
                case bit_counter is
                    when "0000" =>
                        DOUT(0) <= DIN;
                    when "0001" =>
                        DOUT(1) <= DIN;
                    when "0010" =>
                        DOUT(2) <= DIN;                     
                    when "0011" =>
                        DOUT(3) <= DIN;
                    when "0100" =>
                        DOUT(4) <= DIN;
                    when "0101" =>
                        DOUT(5) <= DIN;  
                    when "0110" =>
                        DOUT(6) <= DIN;
                    when "0111" =>
                        DOUT(7) <= DIN;                       
                    when others => null;
                end case;

                -- Increment bit_counter
                bit_counter <= bit_counter + 1;
            end if;

        end if;

    end process;


end architecture;
