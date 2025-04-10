-- uart_rx_fsm.vhd: UART controller - finite state machine controlling RX side
-- Author(s): Marek Pazúr (xpazurm00)

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;



entity UART_RX_FSM is
    port(
      CLK : in std_logic;
      RST : in std_logic;
      DIN : in std_logic;
      
      BIT_COUNTER : in std_logic_vector(3 downto 0);
      CLK_COUNTER : in std_logic_vector(4 downto 0);
      
      CLK_TOGGLE : out std_logic;
      DATA_TRANSFER : out std_logic;
      DATA_VALIDATE : out std_logic
    );
end entity;



architecture behavioral of UART_RX_FSM is
   -- Definition of UART FSM states
   TYPE state IS (IDLE, STANDBY_START_BIT, TRANSFER, STANDBY_STOP_BIT, VALIDATE);
   signal fsm_state : state := IDLE; -- Initial state

begin

   -- UART FSM outputs (in order)
   CLK_TOGGLE <= '0' when fsm_state = IDLE or fsm_state = VALIDATE else '1';

   DATA_TRANSFER <= '1' when fsm_state = TRANSFER else '0';

   DATA_VALIDATE <= '1' when fsm_state = VALIDATE else '0';

   -- Finite state machine logic process sensitive to CLK, RST signals
   fsm_logic_p: process(CLK)
   begin

      -- Asynchronous reset -> sets fsm state to idle (initial state)
      if RST = '1' then
         fsm_state <= IDLE;

      elsif CLK'event and CLK = '1' then

         case fsm_state is
            -- If current state is IDLE and start bit has been detected, set state to STANDBY_START_BIT
            when IDLE =>
               if DIN = '0' then
                  fsm_state <= STANDBY_START_BIT;
               end if;

            -- If current state is STANDBY_START_BIT, wait 23 clock cycles (to take sample from first MIDBIT), then set state to TRANSFER
            when STANDBY_START_BIT =>
               if CLK_COUNTER = "10111" then
                  fsm_state <= TRANSFER;
               end if;

            -- If current state is TRANSFER and 8 bits have been read, set state to STANDBY_STOP_BIT
            when TRANSFER =>
               if BIT_COUNTER = "1000" then
                  fsm_state <= STANDBY_STOP_BIT;
               end if;

            -- If current state is STANDBY_STOP_BIT and DIN is '1', wait 15 clock to take sample from first MIDBIT), then set state to VALIDATE
            when STANDBY_STOP_BIT =>
               if DIN = '1' and CLK_COUNTER = "01111" then
                  fsm_state <= VALIDATE;
               end if;

            -- If current state is VALIDATE, set fsm state back to its initial state (IDLE)
            when VALIDATE =>
               fsm_state <= IDLE;

            -- Revert to initial state if invalid case occured
            when others => 
               fsm_state <= IDLE;
         end case;
      
      end if;

   end process;

end architecture;
