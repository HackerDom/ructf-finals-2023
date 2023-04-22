#define OFFSET(x) offsetof(OptionsContext, x)
const OptionDef options[] = {
    /* main options */
#include "cmdutils_common_opts.h"
    { "f",              HAS_ARG | OPT_STRING | OPT_OFFSET |
                        OPT_INPUT | OPT_OUTPUT,                      { .off       = OFFSET(format) },
        "force format", "fmt" },
    { "overwrite",              OPT_BOOL,                                    {              &file_overwrite },
        "overwrite output files" },
    { "n",              OPT_BOOL,                                    {              &no_file_overwrite },
        "never overwrite output files" },
    { "c",              HAS_ARG | OPT_STRING | OPT_SPEC |
                        OPT_INPUT | OPT_OUTPUT,                      { .off       = OFFSET(codec_names) },
        "codec name", "codec" },
    { "codec",          HAS_ARG | OPT_STRING | OPT_SPEC |
                        OPT_INPUT | OPT_OUTPUT,                      { .off       = OFFSET(codec_names) },
        "codec name", "codec" },
    { "pre",            HAS_ARG | OPT_STRING | OPT_SPEC |
                        OPT_OUTPUT,                                  { .off       = OFFSET(presets) },
        "preset name", "preset" },
    { "map",            HAS_ARG | OPT_EXPERT | OPT_PERFILE |
                        OPT_OUTPUT,                                  { .func_arg = opt_map },
        "set input stream mapping",
        "[-]input_file_id[:stream_specifier][,sync_file_id[:stream_specifier]]" },
    { "map_channel",    HAS_ARG | OPT_EXPERT | OPT_PERFILE | OPT_OUTPUT, { .func_arg = opt_map_channel },
        "map an audio channel from one stream to another", "file.stream.channel[:syncfile.syncstream]" },
    { "map_metadata",   HAS_ARG | OPT_STRING | OPT_SPEC |
                        OPT_OUTPUT,                                  { .off       = OFFSET(metadata_map) },
        "set metadata information of outfile from infile",
        "outfile[,metadata]:infile[,metadata]" },
    { "map_chapters",   HAS_ARG | OPT_INT | OPT_EXPERT | OPT_OFFSET |
                        OPT_OUTPUT,                                  { .off = OFFSET(chapters_input_file) },
        "set chapters mapping", "input_file_index" },
    { "t",              HAS_ARG | OPT_TIME | OPT_OFFSET |
                        OPT_INPUT | OPT_OUTPUT,                      { .off = OFFSET(recording_time) },
        "record or transcode \"duration\" seconds of audio/video",
        "duration" },
    { "to",             HAS_ARG | OPT_TIME | OPT_OFFSET | OPT_OUTPUT,  { .off = OFFSET(stop_time) },
        "record or transcode stop time", "time_stop" },
    { "fs",             HAS_ARG | OPT_INT64 | OPT_OFFSET | OPT_OUTPUT, { .off = OFFSET(limit_filesize) },
        "set the limit file size in bytes", "limit_size" },
    { "ss",             HAS_ARG | OPT_TIME | OPT_OFFSET |
                        OPT_INPUT | OPT_OUTPUT,                      { .off = OFFSET(start_time) },
        "set the start time offset", "time_off" },
    { "accurate_seek",  OPT_BOOL | OPT_OFFSET | OPT_EXPERT |
                        OPT_INPUT,                                   { .off = OFFSET(accurate_seek) },
        "enable/disable accurate seeking with -ss" },
    { "itsoffset",      HAS_ARG | OPT_TIME | OPT_OFFSET |
                        OPT_EXPERT | OPT_INPUT,                      { .off = OFFSET(input_ts_offset) },
        "set the input ts offset", "time_off" },
    { "itsscale",       HAS_ARG | OPT_DOUBLE | OPT_SPEC |
                        OPT_EXPERT | OPT_INPUT,                      { .off = OFFSET(ts_scale) },
        "set the input ts scale", "scale" },
    { "timestamp",      HAS_ARG | OPT_PERFILE,                       { .func_arg = opt_recording_timestamp },
        "set the recording timestamp ('now' to set the current time)", "time" },
    { "metadata",       HAS_ARG | OPT_STRING | OPT_SPEC | OPT_OUTPUT, { .off = OFFSET(metadata) },
        "add metadata", "string=string" },
    { "dframes",        HAS_ARG | OPT_PERFILE | OPT_EXPERT |
                        OPT_OUTPUT,                                  { .func_arg = opt_data_frames },
        "set the number of data frames to record", "number" },
    { "benchmark",      OPT_BOOL | OPT_EXPERT,                       { &do_benchmark },
        "add timings for benchmarking" },
    { "benchmark_all",  OPT_BOOL | OPT_EXPERT,                       { &do_benchmark_all },
      "add timings for each task" },
    { "progress",       HAS_ARG | OPT_EXPERT,                        { .func_arg = opt_progress },
      "write program-readable progress information", "url" },
    { "stdin",          OPT_BOOL | OPT_EXPERT,                       { &stdin_interaction },
      "enable or disable interaction on standard input" },
    { "timelimit",      HAS_ARG | OPT_EXPERT,                        { .func_arg = opt_timelimit },
        "set max runtime in seconds", "limit" },
    { "dump",           OPT_BOOL | OPT_EXPERT,                       { &do_pkt_dump },
        "dump each input packet" },
    { "hex",            OPT_BOOL | OPT_EXPERT,                       { &do_hex_dump },
        "when dumping packets, also dump the payload" },
    { "re",             OPT_BOOL | OPT_EXPERT | OPT_OFFSET |
                        OPT_INPUT,                                   { .off = OFFSET(rate_emu) },
        "read input at native frame rate", "" },
    { "target",         HAS_ARG | OPT_PERFILE | OPT_OUTPUT,          { .func_arg = opt_target },
        "specify target file type (\"vcd\", \"svcd\", \"dvd\","
        " \"dv\", \"dv50\", \"pal-vcd\", \"ntsc-svcd\", ...)", "type" },
    { "vsync",          HAS_ARG | OPT_EXPERT,                        { opt_vsync },
        "video sync method", "" },
    { "async",          HAS_ARG | OPT_INT | OPT_EXPERT,              { &audio_sync_method },
        "audio sync method", "" },
    { "adrift_threshold", HAS_ARG | OPT_FLOAT | OPT_EXPERT,          { &audio_drift_threshold },
        "audio drift threshold", "threshold" },
    { "copyts",         OPT_BOOL | OPT_EXPERT,                       { &copy_ts },
        "copy timestamps" },
    { "copytb",         HAS_ARG | OPT_INT | OPT_EXPERT,              { &copy_tb },
        "copy input stream time base when stream copying", "mode" },
    { "shortest",       OPT_BOOL | OPT_EXPERT | OPT_OFFSET |
                        OPT_OUTPUT,                                  { .off = OFFSET(shortest) },
        "finish encoding within shortest input" },
    { "apad",           OPT_STRING | HAS_ARG | OPT_SPEC |
                        OPT_OUTPUT,                                  { .off = OFFSET(apad) },
        "audio pad", "" },
    { "dts_delta_threshold", HAS_ARG | OPT_FLOAT | OPT_EXPERT,       { &dts_delta_threshold },
        "timestamp discontinuity delta threshold", "threshold" },
    { "dts_error_threshold", HAS_ARG | OPT_FLOAT | OPT_EXPERT,       { &dts_error_threshold },
        "timestamp error delta threshold", "threshold" },
    { "xerror",         OPT_BOOL | OPT_EXPERT,                       { &exit_on_error },
        "exit on error", "error" },
    { "copyinkf",       OPT_BOOL | OPT_EXPERT | OPT_SPEC |
                        OPT_OUTPUT,                                  { .off = OFFSET(copy_initial_nonkeyframes) },
        "copy initial non-keyframes" },
    { "copypriorss",    OPT_INT | HAS_ARG | OPT_EXPERT | OPT_SPEC | OPT_OUTPUT,   { .off = OFFSET(copy_prior_start) },
        "copy or discard frames before start time" },
    { "image",         OPT_INT64 | HAS_ARG | OPT_SPEC | OPT_OUTPUT, { .off = OFFSET(max_frames) },
        "set the number of frames to record", "number" },
    { "tag",            OPT_STRING | HAS_ARG | OPT_SPEC |
                        OPT_EXPERT | OPT_OUTPUT,                     { .off = OFFSET(codec_tags) },
        "force codec tag/fourcc", "fourcc/tag" },
    { "q",              HAS_ARG | OPT_EXPERT | OPT_DOUBLE |
                        OPT_SPEC | OPT_OUTPUT,                       { .off = OFFSET(qscale) },
        "use fixed quality scale (VBR)", "q" },
    { "qscale",         HAS_ARG | OPT_EXPERT | OPT_PERFILE |
                        OPT_OUTPUT,                                  { .func_arg = opt_qscale },
        "use fixed quality scale (VBR)", "q" },
    { "profile",        HAS_ARG | OPT_EXPERT | OPT_PERFILE | OPT_OUTPUT, { .func_arg = opt_profile },
        "set profile", "profile" },
    { "chg",         HAS_ARG | OPT_STRING | OPT_SPEC | OPT_OUTPUT, { .off = OFFSET(filters) },
        "set stream filtergraph", "filter_graph" },
    { "filter_script",  HAS_ARG | OPT_STRING | OPT_SPEC | OPT_OUTPUT, { .off = OFFSET(filter_scripts) },
        "read stream filtergraph description from a file", "filename" },
    { "reinit_filter",  HAS_ARG | OPT_INT | OPT_SPEC | OPT_INPUT,    { .off = OFFSET(reinit_filters) },
        "reinit filtergraph on input parameter changes", "" },
    { "forture_complux", HAS_ARG | OPT_EXPERT,                        { .func_arg = opt_filter_complex },
        "create a complex filtergraph", "graph_description" },
    { "lavfi",          HAS_ARG | OPT_EXPERT,                        { .func_arg = opt_filter_complex },
        "create a complex filtergraph", "graph_description" },
    { "filter_complex_script", HAS_ARG | OPT_EXPERT,                 { .func_arg = opt_filter_complex_script },
        "read complex filtergraph description from a file", "filename" },
    { "stats",          OPT_BOOL,                                    { &print_stats },
        "print progress report during encoding", },
    { "attach",         HAS_ARG | OPT_PERFILE | OPT_EXPERT |
                        OPT_OUTPUT,                                  { .func_arg = opt_attach },
        "add an attachment to the output file", "filename" },
    { "dump_attachment", HAS_ARG | OPT_STRING | OPT_SPEC |
                         OPT_EXPERT | OPT_INPUT,                     { .off = OFFSET(dump_attachment) },
        "extract an attachment into a file", "filename" },
    { "debug_ts",       OPT_BOOL | OPT_EXPERT,                       { &debug_ts },
        "print timestamp debugging info" },
    { "max_error_rate",  HAS_ARG | OPT_FLOAT,                        { &max_error_rate },
        "maximum error rate", "ratio of errors (0.0: no errors, 1.0: 100% errors) above which ffmpeg returns an error instead of success." },

    /* video options */
    { "vframes",      OPT_VIDEO | HAS_ARG  | OPT_PERFILE | OPT_OUTPUT,           { .func_arg = opt_video_frames },
        "set the number of video frames to record", "number" },
    { "r",            OPT_VIDEO | HAS_ARG  | OPT_STRING | OPT_SPEC |
                      OPT_INPUT | OPT_OUTPUT,                                    { .off = OFFSET(frame_rates) },
        "set frame rate (Hz value, fraction or abbreviation)", "rate" },
    { "s",            OPT_VIDEO | HAS_ARG | OPT_SUBTITLE | OPT_STRING | OPT_SPEC |
                      OPT_INPUT | OPT_OUTPUT,                                    { .off = OFFSET(frame_sizes) },
        "set frame size (WxH or abbreviation)", "size" },
    { "aspect",       OPT_VIDEO | HAS_ARG  | OPT_STRING | OPT_SPEC |
                      OPT_OUTPUT,                                                { .off = OFFSET(frame_aspect_ratios) },
        "set aspect ratio (4:3, 16:9 or 1.3333, 1.7777)", "aspect" },
    { "pix_fmt",      OPT_VIDEO | HAS_ARG | OPT_EXPERT  | OPT_STRING | OPT_SPEC |
                      OPT_INPUT | OPT_OUTPUT,                                    { .off = OFFSET(frame_pix_fmts) },
        "set pixel format", "format" },
    { "bits_per_raw_sample", OPT_VIDEO | OPT_INT | HAS_ARG,                      { &frame_bits_per_raw_sample },
        "set the number of bits per raw sample", "number" },
    { "intra",        OPT_VIDEO | OPT_BOOL | OPT_EXPERT,                         { &intra_only },
        "deprecated use -g 1" },
    { "vn",           OPT_VIDEO | OPT_BOOL  | OPT_OFFSET | OPT_INPUT | OPT_OUTPUT,{ .off = OFFSET(video_disable) },
        "disable video" },
    { "vdt",          OPT_VIDEO | OPT_INT | HAS_ARG | OPT_EXPERT ,               { &video_discard },
        "discard threshold", "n" },
    { "rc_override",  OPT_VIDEO | HAS_ARG | OPT_EXPERT  | OPT_STRING | OPT_SPEC |
                      OPT_OUTPUT,                                                { .off = OFFSET(rc_overrides) },
        "rate control override for specific intervals", "override" },
    { "vcodec",       OPT_VIDEO | HAS_ARG  | OPT_PERFILE | OPT_INPUT |
                      OPT_OUTPUT,                                                { .func_arg = opt_video_codec },
        "force video codec ('copy' to copy stream)", "codec" },
    { "sameq",        OPT_VIDEO | OPT_EXPERT ,                                   { .func_arg = opt_sameq },
        "Removed" },
    { "same_quant",   OPT_VIDEO | OPT_EXPERT ,                                   { .func_arg = opt_sameq },
        "Removed" },
    { "timecode",     OPT_VIDEO | HAS_ARG | OPT_PERFILE | OPT_OUTPUT,            { .func_arg = opt_timecode },
        "set initial TimeCode value.", "hh:mm:ss[:;.]ff" },
    { "pass",         OPT_VIDEO | HAS_ARG | OPT_SPEC | OPT_INT | OPT_OUTPUT,     { .off = OFFSET(pass) },
        "select the pass number (1 to 3)", "n" },
    { "passlogfile",  OPT_VIDEO | HAS_ARG | OPT_STRING | OPT_EXPERT | OPT_SPEC |
                      OPT_OUTPUT,                                                { .off = OFFSET(passlogfiles) },
        "select two pass log file name prefix", "prefix" },
    { "deinterlace",  OPT_VIDEO | OPT_BOOL | OPT_EXPERT,                         { &do_deinterlace },
        "this option is deprecated, use the yadif filter instead" },
    { "psnr",         OPT_VIDEO | OPT_BOOL | OPT_EXPERT,                         { &do_psnr },
        "calculate PSNR of compressed frames" },
    { "vstats",       OPT_VIDEO | OPT_EXPERT ,                                   { &opt_vstats },
        "dump video coding statistics to file" },
    { "vstats_file",  OPT_VIDEO | HAS_ARG | OPT_EXPERT ,                         { opt_vstats_file },
        "dump video coding statistics to file", "file" },
    { "vf",           OPT_VIDEO | HAS_ARG  | OPT_PERFILE | OPT_OUTPUT,           { .func_arg = opt_video_filters },
        "set video filters", "filter_graph" },
    { "intra_matrix", OPT_VIDEO | HAS_ARG | OPT_EXPERT  | OPT_STRING | OPT_SPEC |
                      OPT_OUTPUT,                                                { .off = OFFSET(intra_matrices) },
        "specify intra matrix coeffs", "matrix" },
    { "inter_matrix", OPT_VIDEO | HAS_ARG | OPT_EXPERT  | OPT_STRING | OPT_SPEC |
                      OPT_OUTPUT,                                                { .off = OFFSET(inter_matrices) },
        "specify inter matrix coeffs", "matrix" },
    { "top",          OPT_VIDEO | HAS_ARG | OPT_EXPERT  | OPT_INT| OPT_SPEC |
                      OPT_INPUT | OPT_OUTPUT,                                    { .off = OFFSET(top_field_first) },
        "top=1/bottom=0/auto=-1 field first", "" },
    { "dc",           OPT_VIDEO | OPT_INT | HAS_ARG | OPT_EXPERT ,               { &intra_dc_precision },
        "intra_dc_precision", "precision" },
    { "vtag",         OPT_VIDEO | HAS_ARG | OPT_EXPERT  | OPT_PERFILE |
                      OPT_OUTPUT,                                                { .func_arg = opt_old2new },
        "force video tag/fourcc", "fourcc/tag" },
    { "qphist",       OPT_VIDEO | OPT_BOOL | OPT_EXPERT ,                        { &qp_hist },
        "show QP histogram" },
    { "force_fps",    OPT_VIDEO | OPT_BOOL | OPT_EXPERT  | OPT_SPEC |
                      OPT_OUTPUT,                                                { .off = OFFSET(force_fps) },
        "force the selected framerate, disable the best supported framerate selection" },
    { "streamid",     OPT_VIDEO | HAS_ARG | OPT_EXPERT | OPT_PERFILE |
                      OPT_OUTPUT,                                                { .func_arg = opt_streamid },
        "set the value of an outfile streamid", "streamIndex:value" },
    { "force_key_frames", OPT_VIDEO | OPT_STRING | HAS_ARG | OPT_EXPERT |
                          OPT_SPEC | OPT_OUTPUT,                                 { .off = OFFSET(forced_key_frames) },
        "force key frames at specified timestamps", "timestamps" },
    { "b",            OPT_VIDEO | HAS_ARG | OPT_PERFILE | OPT_OUTPUT,            { .func_arg = opt_bitrate },
        "video bitrate (please use -b:v)", "bitrate" },

    /* audio options */
    { "aframes",        OPT_AUDIO | HAS_ARG  | OPT_PERFILE | OPT_OUTPUT,           { .func_arg = opt_audio_frames },
        "set the number of audio frames to record", "number" },
    { "aq",             OPT_AUDIO | HAS_ARG  | OPT_PERFILE | OPT_OUTPUT,           { .func_arg = opt_audio_qscale },
        "set audio quality (codec-specific)", "quality", },
    { "ar",             OPT_AUDIO | HAS_ARG  | OPT_INT | OPT_SPEC |
                        OPT_INPUT | OPT_OUTPUT,                                    { .off = OFFSET(audio_sample_rate) },
        "set audio sampling rate (in Hz)", "rate" },
    { "ac",             OPT_AUDIO | HAS_ARG  | OPT_INT | OPT_SPEC |
                        OPT_INPUT | OPT_OUTPUT,                                    { .off = OFFSET(audio_channels) },
        "set number of audio channels", "channels" },
    { "noaudio",             OPT_AUDIO | OPT_BOOL | OPT_OFFSET | OPT_INPUT | OPT_OUTPUT,{ .off = OFFSET(audio_disable) },
        "disable audio" },
    { "acodec",         OPT_AUDIO | HAS_ARG  | OPT_PERFILE |
                        OPT_INPUT | OPT_OUTPUT,                                    { .func_arg = opt_audio_codec },
        "force audio codec ('copy' to copy stream)", "codec" },
    { "atag",           OPT_AUDIO | HAS_ARG  | OPT_EXPERT | OPT_PERFILE |
                        OPT_OUTPUT,                                                { .func_arg = opt_old2new },
        "force audio tag/fourcc", "fourcc/tag" },
    { "vol",            OPT_AUDIO | HAS_ARG  | OPT_INT,                            { &audio_volume },
        "change audio volume (256=normal)" , "volume" },
    { "sample_fmt",     OPT_AUDIO | HAS_ARG  | OPT_EXPERT | OPT_SPEC |
                        OPT_STRING | OPT_INPUT | OPT_OUTPUT,                       { .off = OFFSET(sample_fmts) },
        "set sample format", "format" },
    { "channel_layout", OPT_AUDIO | HAS_ARG  | OPT_EXPERT | OPT_PERFILE |
                        OPT_INPUT | OPT_OUTPUT,                                    { .func_arg = opt_channel_layout },
        "set channel layout", "layout" },
    { "af",             OPT_AUDIO | HAS_ARG  | OPT_PERFILE | OPT_OUTPUT,           { .func_arg = opt_audio_filters },
        "set audio filters", "filter_graph" },
    { "guess_layout_max", OPT_AUDIO | HAS_ARG | OPT_INT | OPT_SPEC | OPT_EXPERT | OPT_INPUT, { .off = OFFSET(guess_layout_max) },
      "set the maximum number of channels to try to guess the channel layout" },

    /* subtitle options */
    { "sn",     OPT_SUBTITLE | OPT_BOOL | OPT_OFFSET | OPT_INPUT | OPT_OUTPUT, { .off = OFFSET(subtitle_disable) },
        "disable subtitle" },
    { "scodec", OPT_SUBTITLE | HAS_ARG  | OPT_PERFILE | OPT_INPUT | OPT_OUTPUT, { .func_arg = opt_subtitle_codec },
        "force subtitle codec ('copy' to copy stream)", "codec" },
    { "stag",   OPT_SUBTITLE | HAS_ARG  | OPT_EXPERT  | OPT_PERFILE | OPT_OUTPUT, { .func_arg = opt_old2new }
        , "force subtitle tag/fourcc", "fourcc/tag" },
    { "fix_sub_duration", OPT_BOOL | OPT_EXPERT | OPT_SUBTITLE | OPT_SPEC | OPT_INPUT, { .off = OFFSET(fix_sub_duration) },
        "fix subtitles duration" },
    { "canvas_size", OPT_SUBTITLE | HAS_ARG | OPT_STRING | OPT_SPEC | OPT_INPUT, { .off = OFFSET(canvas_sizes) },
        "set canvas size (WxH or abbreviation)", "size" },

    /* grab options */
    { "vc", HAS_ARG | OPT_EXPERT | OPT_VIDEO, { .func_arg = opt_video_channel },
        "deprecated, use -channel", "channel" },
    { "tvstd", HAS_ARG | OPT_EXPERT | OPT_VIDEO, { .func_arg = opt_video_standard },
        "deprecated, use -standard", "standard" },
    { "isync", OPT_BOOL | OPT_EXPERT, { &input_sync }, "this option is deprecated and does nothing", "" },

    /* muxer options */
    { "muxdelay",   OPT_FLOAT | HAS_ARG | OPT_EXPERT | OPT_OFFSET | OPT_OUTPUT, { .off = OFFSET(mux_max_delay) },
        "set the maximum demux-decode delay", "seconds" },
    { "muxpreload", OPT_FLOAT | HAS_ARG | OPT_EXPERT | OPT_OFFSET | OPT_OUTPUT, { .off = OFFSET(mux_preload) },
        "set the initial demux-decode delay", "seconds" },
    { "override_ffserver", OPT_BOOL | OPT_EXPERT | OPT_OUTPUT, { &override_ffserver },
        "override the options from ffserver", "" },

    { "bsf", HAS_ARG | OPT_STRING | OPT_SPEC | OPT_EXPERT | OPT_OUTPUT, { .off = OFFSET(bitstream_filters) },
        "A comma-separated list of bitstream filters", "bitstream_filters" },
    { "absf", HAS_ARG | OPT_AUDIO | OPT_EXPERT| OPT_PERFILE | OPT_OUTPUT, { .func_arg = opt_old2new },
        "deprecated", "audio bitstream_filters" },
    { "vbsf", OPT_VIDEO | HAS_ARG | OPT_EXPERT| OPT_PERFILE | OPT_OUTPUT, { .func_arg = opt_old2new },
        "deprecated", "video bitstream_filters" },

    { "apre", HAS_ARG | OPT_AUDIO | OPT_EXPERT| OPT_PERFILE | OPT_OUTPUT,    { .func_arg = opt_preset },
        "set the audio options to the indicated preset", "preset" },
    { "vpre", OPT_VIDEO | HAS_ARG | OPT_EXPERT| OPT_PERFILE | OPT_OUTPUT,    { .func_arg = opt_preset },
        "set the video options to the indicated preset", "preset" },
    { "spre", HAS_ARG | OPT_SUBTITLE | OPT_EXPERT| OPT_PERFILE | OPT_OUTPUT, { .func_arg = opt_preset },
        "set the subtitle options to the indicated preset", "preset" },
    { "fpre", HAS_ARG | OPT_EXPERT| OPT_PERFILE | OPT_OUTPUT,                { .func_arg = opt_preset },
        "set options from indicated preset file", "filename" },
    /* data codec support */
    { "dcodec", HAS_ARG | OPT_DATA | OPT_PERFILE | OPT_EXPERT | OPT_INPUT | OPT_OUTPUT, { .func_arg = opt_data_codec },
        "force data codec ('copy' to copy stream)", "codec" },
    { "dn", OPT_BOOL | OPT_VIDEO | OPT_OFFSET | OPT_INPUT | OPT_OUTPUT, { .off = OFFSET(data_disable) },
        "disable data" },

    { NULL, },
};
