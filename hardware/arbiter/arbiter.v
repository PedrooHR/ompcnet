`timescale 1s / 1ps

module arbiter #(
    parameter reg [ 7:0] PACKET_SIZE   = 22,
    parameter reg [10:0] MIN_THRESHOLD = 22,
    parameter reg [10:0] MAX_THRESHOLD = 2200,
    parameter reg [10:0] NUM_EXTRA_FIFOS = 4
) (
    input wire aclk,
    input wire aresetn,

    input  wire [511:0] app2net_tx_tdata,
    input  wire [ 63:0] app2net_tx_tkeep,
    input  wire [ 15:0] app2net_tx_tdest,
    input  wire         app2net_tx_tvalid,
    output wire         app2net_tx_tready,
    input  wire         app2net_tx_tlast,

    output wire [511:0] net2app_rx_tdata,
    output wire [ 63:0] net2app_rx_tkeep,
    output wire [ 15:0] net2app_rx_tdest,
    output wire         net2app_rx_tvalid,
    input  wire         net2app_rx_tready,
    output wire         net2app_rx_tlast,

    output wire [511:0] network_tx_tdata,
    output wire [ 63:0] network_tx_tkeep,
    output wire [ 15:0] network_tx_tdest,
    output wire         network_tx_tvalid,
    input  wire         network_tx_tready,
    output wire         network_tx_tlast,

    input  wire [511:0] network_rx_tdata,
    input  wire [ 63:0] network_rx_tkeep,
    input  wire [ 15:0] network_rx_tdest,
    input  wire         network_rx_tvalid,
    output reg          network_rx_tready,
    input  wire         network_rx_tlast
);
  // Params
  localparam reg [15:0] EmptyState = 16'h00FF;
  localparam reg [15:0] FullState = 16'hFF00;

  // Global Controlling
  reg stall_tx = 1'b0;
  reg remote_stall_tx = 1'b0;
  reg empty_msg = 1'b0;
  reg full_msg = 1'b0;
  reg full_msg_pred = 1'b0;
  reg empty_msg_pred = 1'b0;
  reg tx_stall_msg_pred = 1'b0;
  reg tx_stall_pred = 1'b0;
  reg finished_packet = 1'b1;
  reg [15:0] incoming_src = 16'hFFFF;

  // Tx Path Controlling
  wire tx_queue_in_valid;
  wire tx_queue_in_ready;
  wire [511:0] tx_queue_in_data;
  wire [63:0] tx_queue_in_keep;
  wire tx_queue_in_last;
  wire [15:0] tx_queue_in_dest;
  wire tx_queue_out_valid;
  reg tx_queue_out_ready;
  wire [511:0] tx_queue_out_data;
  wire [63:0] tx_queue_out_keep;
  wire tx_queue_out_last;
  wire [15:0] tx_queue_out_dest;

  TxFIFO I_TxFIFO (
      .s_aclk       (aclk),                // input wire s_aclk
      .s_aresetn    (aresetn),             // input wire s_aresetn
      .s_axis_tvalid(tx_queue_in_valid),   // input wire s_axis_tvalid
      .s_axis_tready(tx_queue_in_ready),   // output wire s_axis_tready
      .s_axis_tdata (tx_queue_in_data),    // input wire [511 : 0] s_axis_tdata
      .s_axis_tkeep (tx_queue_in_keep),    // input wire [63 : 0] s_axis_tkeep
      .s_axis_tlast (tx_queue_in_last),    // input wire s_axis_tlast
      .s_axis_tdest (tx_queue_in_dest),    // input wire [15 : 0] s_axis_tdest
      .m_axis_tvalid(tx_queue_out_valid),  // output wire m_axis_tvalid
      .m_axis_tready(tx_queue_out_ready),  // input wire m_axis_tready
      .m_axis_tdata (tx_queue_out_data),   // output wire [511 : 0] m_axis_tdata
      .m_axis_tkeep (tx_queue_out_keep),   // output wire [63 : 0] m_axis_tkeep
      .m_axis_tlast (tx_queue_out_last),   // output wire m_axis_tlast
      .m_axis_tdest (tx_queue_out_dest)    // output wire [15 : 0] m_axis_tdest
  );

  // Input TX logic
  assign tx_queue_in_data  = app2net_tx_tdata;
  assign tx_queue_in_keep  = app2net_tx_tkeep;
  assign tx_queue_in_last  = app2net_tx_tlast;
  assign tx_queue_in_dest  = app2net_tx_tdest;
  assign app2net_tx_tready = tx_queue_in_ready;
  assign tx_queue_in_valid = app2net_tx_tvalid;

  // Output TX logic
  reg [511:0] out_data;
  reg [63:0] out_keep;
  reg [15:0] out_dest;
  reg out_last;
  reg out_valid;
  reg prev_out_valid;

  localparam reg [2:0] TxStateStall = 3'b000;
  localparam reg [2:0] TxStateStallMSG = 3'b010;
  localparam reg [2:0] TxStateData = 3'b100;
  reg [2:0] TxState = 3'b100; // TxStateData

  always @(posedge aclk) begin
    if (!aresetn) begin
      TxState <= TxStateData;

      remote_stall_tx <= 1'b0;
      out_valid <= 1'b0;
      finished_packet <= 1'b1;
    end else begin
      case (TxState)
        TxStateStall: begin
          // Finish an ongoing operation if exist
          if ((network_tx_tready == 1'b1) & (network_tx_tvalid == 1'b1)) begin
            out_valid <= 1'b0;
          end

          if ((full_msg_pred == 1'b1) | (empty_msg_pred == 1'b1)) begin
            TxState <= TxStateStallMSG;
          end else if (stall_tx == 1'b0) begin
            TxState <= TxStateData;
          end
        end
        TxStateStallMSG: begin
          // There might be an operation stalled, wait for it to finish
          if (out_valid == 1'b1) begin
            if ((network_tx_tready == 1'b1) & (network_tx_tvalid == 1'b1)) begin
              out_valid <= 1'b0;
            end
          end else begin
            if (network_tx_tready == 1'b1) begin
              if (full_msg_pred == 1'b1) begin
                remote_stall_tx <= 1'b1;
                out_data <= {{496{1'b0}}, FullState};
                out_keep <= 64'h0000000000FFFFFF;
                out_dest <= incoming_src;
                out_last <= 1'b1;
                out_valid <= 1'b1;

                if (stall_tx == 1'b1) begin
                  TxState <= TxStateStall;
                end else begin
                  TxState <= TxStateData;
                end
              end else if (empty_msg_pred == 1'b1) begin
                remote_stall_tx <= 1'b0;
                out_data <= {{496{1'b0}}, EmptyState};
                out_keep <= 64'h00000000FFFFFFFF;
                out_dest <= incoming_src;
                out_last <= 1'b1;
                out_valid <= 1'b1;

                if (stall_tx == 1'b1) begin
                  TxState <= TxStateStall;
                end else begin
                  TxState <= TxStateData;
                end
              end
            end else begin
              out_valid <= 1'b0;
            end
          end
        end
        TxStateData: begin
          if (tx_stall_msg_pred == 1'b1) begin
            if ((network_tx_tready == 1'b1) & (tx_queue_out_valid == 1'b1)) begin
              out_valid <= 1'b0;
            end

            TxState <= TxStateStallMSG;
          end else if (tx_stall_pred == 1'b1) begin
            if ((network_tx_tready == 1'b1) & (tx_queue_out_valid == 1'b1)) begin
              out_valid <= 1'b0;
            end

            TxState <= TxStateStall;
          end else begin
            if ((network_tx_tready == 1'b1) & (tx_queue_out_valid == 1'b1)) begin
              out_data <= tx_queue_out_data;
              out_keep <= tx_queue_out_keep;
              out_dest <= tx_queue_out_dest;
              out_last <= tx_queue_out_last;
              out_valid <= 1'b1;

              if (tx_queue_out_last == 1'b1) begin
                finished_packet <= 1'b1;
              end else begin
                finished_packet <= 1'b0;
              end
            end else if (network_tx_tready & network_tx_tvalid & !tx_queue_out_valid) begin
              out_valid <= 1'b0;       
            end
          end
        end
        default: begin
          TxState <= TxStateData;
        end
      endcase
    end
  end

  assign network_tx_tdata  = out_data;
  assign network_tx_tkeep  = out_keep;
  assign network_tx_tdest  = out_dest;
  assign network_tx_tlast  = out_last;
  assign network_tx_tvalid = out_valid;
  
  always @(*) begin
    if ((finished_packet == 1'b1) & ((full_msg_pred == 1'b1) | (empty_msg_pred == 1'b1))) begin
      tx_stall_msg_pred <= 1'b1;
    end else begin
      tx_stall_msg_pred <= 1'b0;
    end
  end

  always @(*) begin
    if ((finished_packet == 1'b1) & (stall_tx == 1'b1)) begin
      tx_stall_pred <= 1'b1;
    end else begin
      tx_stall_pred <= 1'b0;
    end
  end

  always @(*) begin
    if ((TxState == TxStateData) & (tx_stall_msg_pred == 1'b0) & (tx_stall_pred == 1'b0)) begin
      tx_queue_out_ready <= network_tx_tready;
    end else begin
      tx_queue_out_ready <= 1'b0;
    end
  end

  always @(*) begin
    if ((aresetn == 1'b1) & (full_msg == 1'b1) & (remote_stall_tx == 1'b0)) begin
      full_msg_pred <= 1'b1;
    end else begin
      full_msg_pred <= 1'b0;
    end
  end

  always @(*) begin
    if ((aresetn == 1'b1) & (empty_msg == 1'b1) & (remote_stall_tx == 1'b1)) begin
      empty_msg_pred <= 1'b1;
    end else begin
      empty_msg_pred <= 1'b0;
    end
  end

  // RX Path Controlling
  reg  rx_queue_ackl_en;
  wire rx_queue_in_valid;
  wire rx_queue_in_ready;
  wire [511:0] rx_queue_in_data;
  wire [63:0] rx_queue_in_keep;
  wire rx_queue_in_last;
  wire [15:0] rx_queue_in_dest;
  wire rx_queue_out_valid;
  wire rx_queue_out_ready;
  wire [511:0] rx_queue_out_data;
  wire [63:0] rx_queue_out_keep;
  wire rx_queue_out_last;
  wire [15:0] rx_queue_out_dest;
  wire rx_queue_full, rx_queue_empty;

  RxFIFO I_RxFIFO (
      .s_aclk                (aclk),
      .s_aresetn             (aresetn),
      .s_aclk_en             (rx_queue_ackl_en), 
      .s_axis_tvalid         (rx_queue_in_valid),
      .s_axis_tready         (rx_queue_in_ready),
      .s_axis_tdata          (rx_queue_in_data),
      .s_axis_tkeep          (rx_queue_in_keep),
      .s_axis_tlast          (rx_queue_in_last),
      .s_axis_tdest          (rx_queue_in_dest),
      .m_axis_tvalid         (rx_queue_out_valid),
      .m_axis_tready         (rx_queue_out_ready),
      .m_axis_tdata          (rx_queue_out_data),
      .m_axis_tkeep          (rx_queue_out_keep),
      .m_axis_tlast          (rx_queue_out_last),
      .m_axis_tdest          (rx_queue_out_dest),
      .axis_prog_full_thresh (MAX_THRESHOLD),
      .axis_prog_empty_thresh(MIN_THRESHOLD),
      .axis_prog_full        (rx_queue_full),
      .axis_prog_empty       (rx_queue_empty)
  );

  // Input RX Logic  
  reg [511:0] in_data;
  reg [63:0] in_keep;
  reg [15:0] in_dest;
  reg in_last;
  reg in_valid;

  assign rx_queue_in_data  = in_data;
  assign rx_queue_in_keep  = in_keep;
  assign rx_queue_in_dest  = in_dest;
  assign rx_queue_in_last  = in_last;
  assign rx_queue_in_valid = in_valid;

  always @(posedge aclk) begin
    if (!aresetn) begin
      rx_queue_ackl_en <= 1'b0;
      in_valid <= 1'b0;
      incoming_src <= 16'd99;
      network_rx_tready <= rx_queue_in_ready;
    end else begin
      if ((network_rx_tready == 1'b1) & (network_rx_tvalid == 1'b1)) begin
        if (network_rx_tkeep == 64'h0000000000FFFFFF) begin
          rx_queue_ackl_en <= 1'b0;

          if (network_rx_tdata[15:0] == FullState) begin
            stall_tx <= 1'b1;
          end
        end else if (network_rx_tkeep == 64'h00000000FFFFFFFF) begin
          rx_queue_ackl_en <= 1'b0;

          if (network_rx_tdata[15:0] == EmptyState) begin
            stall_tx <= 1'b0;
          end
        end else begin
          rx_queue_ackl_en <= 1'b1;

          in_data <= network_rx_tdata;
          in_keep <= network_rx_tkeep;
          in_dest <= network_rx_tdest;
          in_last <= network_rx_tlast;
          in_valid <= network_rx_tvalid;

          if (network_rx_tdest != incoming_src) begin
            incoming_src <= network_rx_tdest;
          end
        end
      end else begin
        rx_queue_ackl_en <= 1'b0;
        in_valid <= 1'b0;
      end
    end

    network_rx_tready <= rx_queue_in_ready;
  end

  // Output RX logic
  wire ef_tvalid[NUM_EXTRA_FIFOS:0];
  wire ef_tready[NUM_EXTRA_FIFOS:0];
  wire [511:0] ef_tdata[NUM_EXTRA_FIFOS:0];
  wire [63:0] ef_tkeep[NUM_EXTRA_FIFOS:0];
  wire ef_tlast[NUM_EXTRA_FIFOS:0];
  wire [15:0] ef_tdest[NUM_EXTRA_FIFOS:0];
  
  generate
    for (genvar I = 0; I < NUM_EXTRA_FIFOS; I = I + 1) begin : gen_FIFOS_I
      fifo_generator_0 ExtraFIFO_I (
        .s_aclk(aclk),                // input wire s_aclk
        .s_aresetn(aresetn),          // input wire s_aresetn
        .s_axis_tvalid(ef_tvalid[I+1]),  // input wire s_axis_tvalid
        .s_axis_tready(ef_tready[I+1]),  // output wire s_axis_tready
        .s_axis_tdata(ef_tdata[I+1]),    // input wire [511 : 0] s_axis_tdata
        .s_axis_tkeep(ef_tkeep[I+1]),    // input wire [63 : 0] s_axis_tkeep
        .s_axis_tlast(ef_tlast[I+1]),    // input wire s_axis_tlast
        .s_axis_tdest(ef_tdest[I+1]),    // input wire [15 : 0] s_axis_tdest
        .m_axis_tvalid(ef_tvalid[I]),  // output wire m_axis_tvalid
        .m_axis_tready(ef_tready[I]),  // input wire m_axis_tready
        .m_axis_tdata(ef_tdata[I]),    // output wire [511 : 0] m_axis_tdata
        .m_axis_tkeep(ef_tkeep[I]),    // output wire [63 : 0] m_axis_tkeep
        .m_axis_tlast(ef_tlast[I]),    // output wire m_axis_tlast
        .m_axis_tdest(ef_tdest[I])    // output wire [15 : 0] m_axis_tdest
      );
    end
  endgenerate

  assign ef_tvalid[NUM_EXTRA_FIFOS] = rx_queue_out_valid;
  assign rx_queue_out_ready = ef_tready[NUM_EXTRA_FIFOS];
  assign ef_tdata[NUM_EXTRA_FIFOS] = rx_queue_out_data;
  assign ef_tkeep[NUM_EXTRA_FIFOS] = rx_queue_out_keep;
  assign ef_tlast[NUM_EXTRA_FIFOS] = rx_queue_out_last;
  assign ef_tdest[NUM_EXTRA_FIFOS] = rx_queue_out_dest;


  assign net2app_rx_tdata = ef_tdata[0];
  assign net2app_rx_tkeep = ef_tkeep[0];
  assign net2app_rx_tlast = ef_tlast[0];
  assign net2app_rx_tdest = ef_tdest[0];
  assign ef_tready[0] = net2app_rx_tready; 
  assign net2app_rx_tvalid = ef_tvalid[0]; 

  // Directly connect
  /*assign net2app_rx_tdata   = rx_queue_out_data;
  assign net2app_rx_tkeep   = rx_queue_out_keep;
  assign net2app_rx_tlast   = rx_queue_out_last;
  assign net2app_rx_tdest   = rx_queue_out_dest;
  assign rx_queue_out_ready = net2app_rx_tready;
  assign net2app_rx_tvalid  = rx_queue_out_valid;*/

  // Stall Msg logic
  always @(posedge aclk) begin
    if (!aresetn) begin
      empty_msg <= 1'b0;
      full_msg  <= 1'b0;
    end else if (rx_queue_empty) begin
      empty_msg <= 1'b1;
    end else if (rx_queue_full) begin
      full_msg <= 1'b1;
    end else begin
      empty_msg <= 1'b0;
      full_msg  <= 1'b0;
    end
  end

endmodule
