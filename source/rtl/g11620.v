`timescale 1ns/1ps
`include "param.h"

module g11620 # (
    parameter PIX_NUM = 9'd511
)
(
    input           clk,
    input           rst_n,
/*
    input           start_in, // Start the integration
    input [9:0]     integ_time_in,
    input           set_integ_time,
*/
    output reg      reset_o,
    output          g11620_clk, // MAX fre is 5 MHz
    input           ad_sp,

    // control RAM
    input wire      start_in,
    input wire      soft_reset_in,
    output reg      done_o,
    output wire       cfg_ram_rd_o,
    output wire [7:0] cfg_ram_addr_o,
    input [31:0]     cfg_ram_din
);

localparam CTRL_R_ADDR = 4'd0,
            INTEG_R_ADDR = 4'd1,
            STP_R_ADDR = 4'd2,
            PW_R_ADDR = 4'd3,
            SCAN_R_ADDR = 4'd4,
            SP_R_ADDR = 4'd5,
            EP_R_ADDR = 4'd6;

localparam IDLE = 3'd0,
            GET_INTEG_TIME = 3'd1,
            GET_CAP_TIME = 3'd2,
            INTEG = 3'd3,
            NOP = 3'd4,
            DATA = 3'd5,
            BLANK = 3'd6,
            DONE = 3'd7;

reg [2:0]       state;
reg [31:0]      integ_time_reg;
reg [31:0]      cap_time_reg;
reg [31:0]      cap_time_cnt;
reg [8:0]       adc_data_cnt;
reg [31:0]      clk_cnt;
reg             start_r;
wire            start;
reg [31:0]      ram_din;
reg             ram_wr;
reg [3:0]       ram_addr;
wire [31:0]     ram_dout;
wire            soft_reset;

reg [31:0] ram[0:15];

assign g11620_clk = ~clk;

always @(posedge clk) begin
    start_r <= start_in;
end

always @(posedge clk) begin
    if (~rst_n) begin
        state <= IDLE;
        reset_o <= 1'b0;
    end // if (~rst_n)
    else begin
        reset_o <= 1'b0;
       // cfg_ram_rd_o <= 1'b0;
        done_o <= 1'b0;
        case(state)
            IDLE: begin
                if (start_in == 1'b1 && start_r == 1'b0) begin
                    state <= GET_INTEG_TIME;
                   // cfg_ram_rd_o <= 1'b1;
                   // cfg_ram_addr_o <= `G11620_INTEG_R_ADDR;
                end // if (start == 1'b1 && start_r == 1'b0)
                clk_cnt <= 'h0;
                reset_o <= 1'b0;
                adc_data_cnt <= 'h0;
                cap_time_cnt <= 'h0;
            end // IDLE:
            GET_INTEG_TIME: begin
                //state <= INTEG;
                state <= GET_CAP_TIME;
                integ_time_reg <= cfg_ram_din - 1'b1;
              //  cfg_ram_rd_o <= 1'b1;
               // cfg_ram_addr_o <= `G11620_CAP_R_ADDR;
            end // GET_INTEG_TIME:
            GET_CAP_TIME: begin
                cap_time_reg <= cfg_ram_din - 1'b1;
                state <= INTEG;
            end
            INTEG: begin
                reset_o <= 1'b1;
                clk_cnt <= clk_cnt + 1'b1;
                if (clk_cnt == integ_time_reg) begin
                    state <= NOP;
                    clk_cnt <= 'h0;
                end // if (clk_cnt == integ_time_reg)

                if (soft_reset) state <= IDLE;
            end // INTEG:
            NOP: begin
                // trig a timer
                if (ad_sp) state <= DATA;
               else if (soft_reset) state <= IDLE;
            end // NOP:
            DATA: begin
                adc_data_cnt <= adc_data_cnt + 1'd1;
                if (adc_data_cnt == PIX_NUM) begin
                    state <= BLANK;
                end
                if (soft_reset) state <= IDLE;
            end // DATA:
            BLANK: begin
                clk_cnt <= clk_cnt + 1'd1;
               // done_o <= 1'b1;
                if (clk_cnt == 32'd23) begin
                    clk_cnt <= 'h0;
                    if (cap_time_cnt == cap_time_reg) begin
                        state <= DONE;
                    end
                    else begin
                        cap_time_cnt <= cap_time_cnt + 1'b1;
                        state <= INTEG;
                    end
                end // if (clk_cnt == 10'd23)

                if (soft_reset) state <= IDLE;
            end // BLANK:
            DONE: begin
                done_o <= 1'b1;
                clk_cnt <= clk_cnt + 1'b1;
                if (clk_cnt == 32'd32)
                    state <= IDLE;
            end
            default: begin
                state <= IDLE;
            end // default:
        endcase // state
    end // else
end // always @(posedge clk)
assign cfg_ram_rd_o = (state == IDLE || state == GET_INTEG_TIME) ? 1'b1: 1'b0;
assign cfg_ram_addr_o = (state == IDLE) ? `G11620_INTEG_R_ADDR :
                        (state == GET_INTEG_TIME) ? `G11620_CAP_R_ADDR :
                        'h0;

// ---- Debug ----------------
wire [0:0] reset_ila;
wire [0:0] start_ila;
wire [0:0] ad_sp_ila;
wire [0:0] ram_rd_ila;


assign reset_ila[0] = reset_o;
assign start_ila[0] = start_in;
assign ad_sp_ila[0] = ad_sp;
//assign ram_rd_ila[0] = cfg_ram_rd_o;
assign ram_rd_ila[0] = 1'b0;

ila_g11620 ila_g11620_inst (
    .clk(clk), // input wire clk
    .probe0(state), // input wire [2:0]  probe0
    .probe1(reset_ila), // input wire [0:0]  probe1
    .probe2(start_ila), // input wire [0:0]  probe2
    .probe3(clk_cnt), // input wire [9:0]  probe3
    .probe4(adc_data_cnt), // input wire [8:0]  probe4
    .probe5(done_o),
    .probe6(cfg_ram_din),
    .probe7(cfg_ram_rd_o), // input wire [0:0]  probe7
    .probe8(cfg_ram_addr_o), // input wire [7:0]  probe8
    .probe9(integ_time_reg)
);

endmodule

