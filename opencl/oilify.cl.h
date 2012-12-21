static const char* oilify_cl_source =
"/* two small different kernels are better than one big */                     \n"
"                                                                              \n"
"#define NUM_INTENSITIES 256                                                   \n"
"                                                                              \n"
"kernel void kernel_oilify(global float4 *in,                                  \n"
"                             global float4 *out,                              \n"
"                             const int mask_radius,                           \n"
"                             const int intensities,                           \n"
"                             const float exponent)                            \n"
"{                                                                             \n"
"  int gidx = get_global_id(0);                                                \n"
"  int gidy = get_global_id(1);                                                \n"
"  int x = gidx + mask_radius;                                                 \n"
"  int y = gidy + mask_radius;                                                 \n"
"  int dst_width = get_global_size(0);                                         \n"
"  int src_width = dst_width + mask_radius * 2;                                \n"
"  float4 hist[NUM_INTENSITIES];                                               \n"
"  float4 hist_max = 1.0;                                                      \n"
"  int i, j, intensity;                                                        \n"
"  int radius_sq = mask_radius * mask_radius;                                  \n"
"  float4 temp_pixel;                                                          \n"
"  for (i = 0; i < intensities; i++)                                           \n"
"    hist[i] = 0.0;                                                            \n"
"                                                                              \n"
"  for (i = -mask_radius; i <= mask_radius; i++)                               \n"
"  {                                                                           \n"
"    for (j = -mask_radius; j <= mask_radius; j++)                             \n"
"      {                                                                       \n"
"        if (i*i + j*j <= radius_sq)                                           \n"
"          {                                                                   \n"
"            temp_pixel = in[x + i + (y + j) * src_width];                     \n"
"            hist[(int)(temp_pixel.x * (intensities - 1))].x+=1;               \n"
"            hist[(int)(temp_pixel.y * (intensities - 1))].y+=1;               \n"
"            hist[(int)(temp_pixel.z * (intensities - 1))].z+=1;               \n"
"            hist[(int)(temp_pixel.w * (intensities - 1))].w+=1;               \n"
"          }                                                                   \n"
"      }                                                                       \n"
"  }                                                                           \n"
"                                                                              \n"
"  for (i = 0; i < intensities; i++) {                                         \n"
"    if(hist_max.x < hist[i].x)                                                \n"
"      hist_max.x = hist[i].x;                                                 \n"
"    if(hist_max.y < hist[i].y)                                                \n"
"      hist_max.y = hist[i].y;                                                 \n"
"    if(hist_max.z < hist[i].z)                                                \n"
"      hist_max.z = hist[i].z;                                                 \n"
"    if(hist_max.w < hist[i].w)                                                \n"
"      hist_max.w = hist[i].w;                                                 \n"
"  }                                                                           \n"
"  float4 div = 0.0;                                                           \n"
"  float4 sum = 0.0;                                                           \n"
"  float4 ratio, weight;                                                       \n"
"  for (i = 0; i < intensities; i++)                                           \n"
"  {                                                                           \n"
"    ratio = hist[i] / hist_max;                                               \n"
"    weight = pow(ratio, (float4)exponent);                                    \n"
"    sum += weight * (float4)i;                                                \n"
"    div += weight;                                                            \n"
"  }                                                                           \n"
"  out[gidx + gidy * dst_width] = sum / div / (float)(intensities - 1);        \n"
"}                                                                             \n"
"                                                                              \n"
"kernel void kernel_oilify_inten(global float4 *in,                            \n"
"                             global float4 *out,                              \n"
"                             const int mask_radius,                           \n"
"                             const int intensities,                           \n"
"                             const float exponent)                            \n"
"{                                                                             \n"
"  int gidx = get_global_id(0);                                                \n"
"  int gidy = get_global_id(1);                                                \n"
"  int x = gidx + mask_radius;                                                 \n"
"  int y = gidy + mask_radius;                                                 \n"
"  int dst_width = get_global_size(0);                                         \n"
"  int src_width = dst_width + mask_radius * 2;                                \n"
"  float4 cumulative_rgb[NUM_INTENSITIES];                                     \n"
"  int hist_inten[NUM_INTENSITIES], inten_max;                                 \n"
"  int i, j, intensity;                                                        \n"
"  int radius_sq = mask_radius * mask_radius;                                  \n"
"  float4 temp_pixel;                                                          \n"
"  for (i = 0; i < intensities; i++)                                           \n"
"  {                                                                           \n"
"    hist_inten[i] = 0;                                                        \n"
"    cumulative_rgb[i] = 0.0;                                                  \n"
"  }                                                                           \n"
"  for (i = -mask_radius; i <= mask_radius; i++)                               \n"
"  {                                                                           \n"
"    for (j = -mask_radius; j <= mask_radius; j++)                             \n"
"      {                                                                       \n"
"        if (i*i + j*j <= radius_sq)                                           \n"
"          {                                                                   \n"
"            temp_pixel = in[x + i + (y + j) * src_width];                     \n"
"            /*Calculate intensity on the fly, GPU does it fast*/              \n"
"            intensity = (int)((0.299 * temp_pixel.x                           \n"
"                      +0.587 * temp_pixel.y                                   \n"
"                      +0.114 * temp_pixel.z) * (float)(intensities-1));       \n"
"            hist_inten[intensity] += 1;                                       \n"
"            cumulative_rgb[intensity] += temp_pixel;                          \n"
"          }                                                                   \n"
"      }                                                                       \n"
"  }                                                                           \n"
"  inten_max = 1;                                                              \n"
"                                                                              \n"
"  /* calculated maximums */                                                   \n"
"  for (i = 0; i < intensities; i++) {                                         \n"
"    if(hist_inten[i] > inten_max)                                             \n"
"      inten_max = hist_inten[i];                                              \n"
"  }                                                                           \n"
"  float div = 0.0;                                                            \n"
"  float ratio, weight, mult_inten;                                            \n"
"                                                                              \n"
"  float4 color = 0.0;                                                         \n"
"  for (i = 0; i < intensities; i++)                                           \n"
"  {                                                                           \n"
"    if (hist_inten[i] > 0)                                                    \n"
"    {                                                                         \n"
"      ratio = (float)(hist_inten[i]) / (float)(inten_max);                    \n"
"      weight = pow(ratio, exponent);                                          \n"
"      mult_inten = weight / (float)(hist_inten[i]);                           \n"
"                                                                              \n"
"      div += weight;                                                          \n"
"      color += mult_inten * cumulative_rgb[i];                                \n"
"    }                                                                         \n"
"  }                                                                           \n"
"  out[gidx + gidy * dst_width] = color/div;                                   \n"
"}                                                                             \n"
;