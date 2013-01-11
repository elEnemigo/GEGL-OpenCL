static const char* value_invert_cl_source =
"__kernel void gegl_value_invert (__global const float4     *in,               \n"
"                                 __global       float4     *out)              \n"
"{                                                                             \n"
"  int gid = get_global_id(0);                                                 \n"
"  float4 in_v  = in[gid];                                                     \n"
"  float4 out_v;                                                               \n"
"                                                                              \n"
"  float value = fmax (in_v.x, fmax (in_v.y, in_v.z));                         \n"
"  float minv  = fmin (in_v.x, fmin (in_v.y, in_v.z));                         \n"
"  float delta = value - minv;                                                 \n"
"                                                                              \n"
"  if (value == 0.0f || delta == 0.0f)                                         \n"
"    {                                                                         \n"
"      out_v = (float4) ((1.0f - value),                                       \n"
"                        (1.0f - value),                                       \n"
"                        (1.0f - value),                                       \n"
"                        in_v.w);                                              \n"
"    }                                                                         \n"
"  else                                                                        \n"
"    {                                                                         \n"
"      out_v = (float4) ((1.0f - value) * in_v.x / value,                      \n"
"                        (1.0f - value) * in_v.y / value,                      \n"
"                        (1.0f - value) * in_v.z / value,                      \n"
"                        in_v.w);                                              \n"
"    }                                                                         \n"
"                                                                              \n"
"  out_v.w   =  in_v.w;                                                        \n"
"  out[gid]  =  out_v;                                                         \n"
"}                                                                             \n"
"                                                                              \n"
;
