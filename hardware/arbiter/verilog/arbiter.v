`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: LSC - Unicamp
// Engineer: Pedro Henrique Di Francia Rosso
// 
// Create Date: 09/08/2024 10:55:19 PM
// Design Name: 
// Module Name: arbiter
// Project Name: OMPCNet
// Target Devices: Alveo u55c
// Tool Versions: Vitis/Vivado 2022.1
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////

module arbiter (
        input wire ap_clk,
        input wire ap_rst_n,
        
        /* DATA FROM APPLICATION */
        input wire [511:0] app2net_data_tx_TDATA,
        input wire [63:0] app2net_data_tx_TKEEP,
        input wire [63:0] app2net_data_tx_TSTRB,
        input wire [0:0] app2net_data_tx_TLAST,
        input wire [15:0] app2net_data_tx_TDEST,
        input wire app2net_data_tx_TVALID,
        output wire app2net_data_tx_TREADY,
        
        /* DATA TO APPLICATION */
        output wire [511:0] net2app_data_rx_TDATA,'
        output wire [63:0] net2app_data_rx_TKEEP,
        output wire [63:0] net2app_data_rx_TSTRB,
        output wire [0:0] net2app_data_rx_TLAST,
        output wire [15:0] net2app_data_rx_TDEST,
        input wire net2app_data_rx_TVALID,
        output wire net2app_data_rx_TREADY,
        
        /* NETWORK DATA */
        output wire [511:0] network_tx_TDATA,
        output wire [63:0] network_tx_TKEEP,
        output wire [63:0] network_tx_TSTRB,
        output wire [0:0] network_tx_TLAST,
        output wire [15:0] network_tx_TDEST,
        input wire network_tx_TVALID,
        output wire network_tx_TREADY,
        
        input wire [511:0] network_rx_TDATA,
        input wire [63:0] network_rx_TKEEP,
        input wire [63:0] network_rx_TSTRB,
        input wire [0:0] network_rx_TLAST,
        input wire [15:0] network_rx_TDEST,
        input wire network_rx_TVALID,
        output wire network_rx_TREADY,
        
        /* APP2NET HANDSHAKE */
        input wire [511:0] app2net_hs_tx_TDATA,
        input wire [63:0] app2net_hs_tx_TKEEP,
        input wire [63:0] app2net_hs_tx_TSTRB,
        input wire [0:0] app2net_hs_tx_TLAST,
        input wire [15:0] app2net_hs_tx_TDEST,
        input wire app2net_hs_tx_TVALID,
        output wire app2net_hs_tx_TREADY,
        
        output wire [511:0] app2net_hs_rx_TDATA,
        output wire [63:0] app2net_hs_rx_TKEEP,
        output wire [63:0] app2net_hs_rx_TSTRB,
        output wire [0:0] app2net_hs_rx_TLAST,
        output wire [15:0] app2net_hs_rx_TDEST,
        input wire app2net_hs_rx_TVALID,
        output wire app2net_hs_rx_TREADY,
        
        /* NET2APP HANDSHAKE */
        input wire [511:0] net2app_hs_tx_TDATA,
        input wire [63:0] net2app_hs_tx_TKEEP,
        input wire [63:0] net2app_hs_tx_TSTRB,
        input wire [0:0] net2app_hs_tx_TLAST,
        input wire [15:0] net2app_hs_tx_TDEST,
        input wire net2app_hs_tx_TVALID,
        output wire net2app_hs_tx_TREADY,

        output wire [511:0] net2app_hs_rx_TDATA,
        output wire [63:0] net2app_hs_rx_TKEEP,
        output wire [63:0] net2app_hs_rx_TSTRB,
        output wire [0:0] net2app_hs_rx_TLAST,
        output wire [15:0] net2app_hs_rx_TDEST,
        input wire net2app_hs_rx_TVALID,
        output wire net2app_hs_rx_TREADY 
);



















endmodule
