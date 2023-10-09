// OLED Main Controller
/*
Inputs
clock   - 100 MHz clock from on-board Oscillator
reset   - reset push button
d_valid - Validation of sent data
sdin    - data received

Outputs
oled_spi_clk  - 10 MHz clock for OLED // wired from spi_controller
oled_spi_data - channel to send data // wired from spi_controller
oled_vdd      - Parameter for OLED Initializing sequence
oled_vbat     - Parameter for OLED Initializing sequence
oled_reset_n  - Parameter for OLED Initializing sequence
oled_dc_n     - Parameter for OLED Initializing sequence
txDone        - data transmission is completed
*/

// Module Declaration
module oled_controller(
    input  clock, reset, d_valid,
    input [6:0] sdin,
    output wire oled_spi_clk, oled_spi_data,
    output reg oled_vdd, oled_vbat, oled_reset_n, oled_dc_n, txDone
);

wire [63:0] charBitMap;
reg [7:0] spiData, columnAddr;
reg [4:0] state, next;
reg [3:0] byteCounter;
reg [1:0] currPage;
reg startDelay, spiLoadData;
wire delayDone, spiDone;

// Declaring paramters for Finite State Machine
parameter   IDLE = 'd0, DELAY = 'd1, INIT  = 'd2, RESET = 'd3, CHRG_PUMP = 'd4,
            CHRG_PUMP1 = 'd5, WAIT_SPI = 'd6, PRE_CHRG = 'd7,
            PRE_CHRG1 = 'd8, VBAT_ON = 'd9, CONTRAST = 'd10,
            CONTRAST1 = 'd11, SEG_REMAP = 'd12, SCAN_DIR = 'd13,
            COM_PIN = 'd14, COM_PIN1 = 'd15, DISPLAY_ON = 'd16,
            FULL_DISPLAY = 'd17, DONE = 'd18, PAGE_ADDR = 'd19,
            PAGE_ADDR1 = 'd20, PAGE_ADDR2 = 'd21, COLUMN_ADDR = 'd22,
            SEND_DATA = 'd23;

// Initializing Sequence are handled by the FSM
// Along with Mapping text to the OLED.
always @(posedge clock) begin
    if(reset) begin
        state <= IDLE; next <= IDLE; oled_vdd <= 1'b1;
        oled_vbat <= 1'b1; oled_reset_n <= 1'b1; oled_dc_n <= 1'b1;
        startDelay <= 1'b0; spiData <= 8'd0; spiLoadData <= 1'b0;
        currPage <= 0; txDone <= 0; columnAddr <= 0;
    end
    else
    begin
        case(state)
            IDLE:begin
                oled_vbat <= 1'b1; oled_vdd <= 1'b0; state <= DELAY;
                oled_dc_n <= 1'b0; oled_reset_n <= 1'b1; next <= INIT;
            end
            DELAY:begin
                startDelay <= 1'b1;
                if(delayDone) begin
                    state <= next; startDelay <= 1'b0;
                end
            end
            INIT:begin
                spiData <= 'hAE; spiLoadData <= 1'b1;
                if(spiDone) begin
                    spiLoadData <= 1'b0; oled_reset_n <= 1'b0;
                    state <= DELAY; next <= RESET;
                end
            end
            RESET:begin
                 oled_reset_n <= 1'b1; state <= DELAY; next <= CHRG_PUMP;
            end
            CHRG_PUMP:begin
                spiData <= 'h8D; spiLoadData <= 1'b1;
                if(spiDone) begin
                    spiLoadData <= 1'b0; state <= WAIT_SPI; next <= CHRG_PUMP1;
                end
            end
            WAIT_SPI:begin
                if(!spiDone) state <= next;
            end
            CHRG_PUMP1:begin
                spiData <= 'h14; spiLoadData <= 1'b1;
                if(spiDone) begin
                    spiLoadData <= 1'b0; state <= WAIT_SPI; next <= PRE_CHRG;
                end
            end
            PRE_CHRG:begin
                spiData <= 'hD9; spiLoadData <= 1'b1;
                if(spiDone) begin
                    spiLoadData <= 1'b0; state <= WAIT_SPI; next <= PRE_CHRG1;
                end
            end
            PRE_CHRG1:begin
               spiData <= 'hF1; spiLoadData <= 1'b1;
               if(spiDone) begin
                   spiLoadData <= 1'b0; state <= WAIT_SPI; next <= VBAT_ON;
               end
            end            
            VBAT_ON:begin
                oled_vbat <= 1'b0; state <= DELAY; next <= CONTRAST;
            end
            CONTRAST:begin
               spiData <= 'h81; spiLoadData <= 1'b1;
               if(spiDone) begin
                   spiLoadData <= 1'b0; state <= WAIT_SPI; next <= CONTRAST1;
               end
            end  
            CONTRAST1:begin
               spiData <= 'hFF; spiLoadData <= 1'b1;
               if(spiDone) begin
                   spiLoadData <= 1'b0; state <= WAIT_SPI; next <= SEG_REMAP;
               end
            end            
            SEG_REMAP:begin
               spiData <= 'hA0; spiLoadData <= 1'b1;
               if(spiDone) begin
                   spiLoadData <= 1'b0; state <= WAIT_SPI; next <= SCAN_DIR;
               end
            end 
            SCAN_DIR:begin
               spiData <= 'hC0; spiLoadData <= 1'b1;
               if(spiDone) begin
                   spiLoadData <= 1'b0; state <= WAIT_SPI; next <= COM_PIN;
               end
            end 
            COM_PIN:begin
               spiData <= 'hDA; spiLoadData <= 1'b1;
               if(spiDone) begin
                   spiLoadData <= 1'b0; state <= WAIT_SPI; next <= COM_PIN1;
               end
            end
            COM_PIN1:begin
               spiData <= 'h00; spiLoadData <= 1'b1;
               if(spiDone)
               begin
                   spiLoadData <= 1'b0; state <= WAIT_SPI; next <= DISPLAY_ON;
               end
            end  
            DISPLAY_ON:begin
               spiData <= 'hAF; spiLoadData <= 1'b1;
               if(spiDone)
               begin
                   spiLoadData <= 1'b0; state <= WAIT_SPI; next <= PAGE_ADDR;//FULL_DISPLAY;
               end
            end 
            PAGE_ADDR:begin
                spiData <= 'h22; spiLoadData <= 1'b1; oled_dc_n <= 1'b0;
                if(spiDone) begin
                    spiLoadData <= 1'b0; state <= WAIT_SPI; next <= PAGE_ADDR1;
                end
            end
            PAGE_ADDR1:begin
                //start page address
                spiData <= currPage; spiLoadData <= 1'b1;
                if(spiDone) begin
                    spiLoadData <= 1'b0; state <= WAIT_SPI;
                    currPage <= currPage+1; next <= PAGE_ADDR2;
                end
            end  
            PAGE_ADDR2:begin
                //start page address
                spiData <= currPage; spiLoadData <= 1'b1;
                if(spiDone) begin
                    spiLoadData <= 1'b0; state <= WAIT_SPI; next <= COLUMN_ADDR;
                end
            end              
            COLUMN_ADDR:begin
                spiData <= 'h10; spiLoadData <= 1'b1;
                if(spiDone) begin
                    spiLoadData <= 1'b0; state <= WAIT_SPI; next <= DONE;
                end
            end            
            DONE:begin
                txDone <= 1'b0;
                if(d_valid & columnAddr != 128 & !txDone) begin
                    state <= SEND_DATA;
                    byteCounter <= 8;
                end
                else if(d_valid & columnAddr == 128 & !txDone) begin
                    state <= PAGE_ADDR; columnAddr <= 0;  byteCounter <= 8;
                end
            end   
            SEND_DATA:begin
                spiData <= charBitMap[(byteCounter*8-1)-:8];
                spiLoadData <= 1'b1; oled_dc_n <= 1'b1;
                if(spiDone) begin
                    columnAddr <= columnAddr + 1;
                    spiLoadData <= 1'b0; state <= WAIT_SPI;
                    if(byteCounter != 1) begin
                        byteCounter <= byteCounter - 1; next <= SEND_DATA;
                    end
                    else begin
                        next <= DONE; txDone <= 1'b1;
                    end
                end
            end
        endcase
    end
end

// Instantiate Delay Generator
delay_generator DG(.clock(clock), .en_delay(startDelay), .delay_done(delayDone)); 

// Instantiate SPI Controller
spi_controller SC(
    .clock(clock), .reset(reset), .din(spiData),
    .load_data(spiLoadData), .msg_done(spiDone),
    .spi_clk(oled_spi_clk), .spi_data(oled_spi_data)
);

// Add characters to display into the Graphics Display Random Access Memory(GDRAM)
char_ROM CR(.addr(sdin), .data(charBitMap));

endmodule
