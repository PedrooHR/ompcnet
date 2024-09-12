`timescale 1ns / 1ps

module tx_engine # (
  parameter  = ;
  ) (
        input wire ap_clk,
        input wire ap_rst_n,
        
        /* DATA OUT PATH */
        input wire [511:0] app2net_data_tx_TDATA,
        input wire [63:0] app2net_data_tx_TKEEP,
        input wire [63:0] app2net_data_tx_TSTRB,
        input wire [0:0] app2net_data_tx_TLAST,
        input wire [15:0] app2net_data_tx_TDEST,
        input wire app2net_data_tx_TVALID,
        output wire app2net_data_tx_TREADY,

        output wire [511:0] network_tx_TDATA,
        output wire [63:0] network_tx_TKEEP,
        output wire [63:0] network_tx_TSTRB,
        output wire [0:0] network_tx_TLAST,
        output wire [15:0] network_tx_TDEST,
        input wire network_tx_TVALID,
        output wire network_tx_TREADY,
        
        /* HANDSHAKES */
        input wire [511:0] app2net_hs_tx_TDATA,
        input wire [63:0] app2net_hs_tx_TKEEP,
        input wire [63:0] app2net_hs_tx_TSTRB,
        input wire [0:0] app2net_hs_tx_TLAST,
        input wire [15:0] app2net_hs_tx_TDEST,
        input wire app2net_hs_tx_TVALID,
        output wire app2net_hs_tx_TREADY,
        
        input wire [511:0] net2app_hs_tx_TDATA,
        input wire [63:0] net2app_hs_tx_TKEEP,
        input wire [63:0] net2app_hs_tx_TSTRB,
        input wire [0:0] net2app_hs_tx_TLAST,
        input wire [15:0] net2app_hs_tx_TDEST,
        input wire net2app_hs_tx_TVALID,
        output wire net2app_hs_tx_TREADY,

      input wire stall

);

  // control between data transmission and 
  reg [4:0] read_packets = 5'd0; 

  // enable/disable transmission
  wire en_transmission;
  always @(*) begin
    // only stall if not in the middle of a packet
    if (stall && read_packets == 5'd0 && !ap_rst_n) begin
      en_transmission = 1;
    else
      en_transmission = 0;
    end
  end

  // data dispatch control
  reg [1:0] send_control = 3b000;
  always @(posedge ap_clk ) begin
    if (en_transmission && send_control == 3b000) begin
      // only sends if not stalled and not in progress of sending some data
      if (read_packets == 5'd0) begin
        if (app2net_hs_tx_TVALID) begin
          send_control <= 3b001;
        end else if (net2app_hs_tx_TVALID) begin
          send_control <= 3b010;
        end
      end else if (app2net_data_tx_TVALID) begin
        send_control <= 3b100;
        read_packets <= read_packets + 1'b1;
      end
    end
  end

  always @(*) begin
    case (send_control)
      3b001: begin
        app2net_hs_tx_TREADY = 1;
      end
      3b010: begin
        net2app_hs_tx_TREADY = 1;
      end
      3b100: begin
        app2net_data_tx_TREADY = 1;
      end
      default: 
    endcase
  end








endmodule
