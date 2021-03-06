//
// Generated by Microsoft (R) HLSL Shader Compiler 9.29.952.3111
//
// Parameters:
//
//   float4 g_Ambient;
//   float4 g_ChrPlayableMenuParam;
//   float4 g_Diffuse;
//   float4 g_EyeDirection;
//   float4 g_EyePosition;
//   float4 g_ForceAlphaColor;
//   float4 g_GI0Scale;
//   float4 g_LightScatteringColor;
//   float4 g_OpacityReflectionRefractionSpectype;
//   float4 g_PowerGlossLevel;
//   float4 g_ShadowMapParams;
//   sampler2D g_ShadowMapSampler;
//   float4 g_SonicSkinFalloffParam;
//   float4 g_Specular;
//   sampler2D g_VerticalShadowMapSampler;
//   float4 g_aLightField[6];
//   float4 mrgEyeLight_Attribute;
//   float4 mrgEyeLight_Diffuse;
//   float4 mrgEyeLight_Range;
//   float4 mrgEyeLight_Specular;
//   float4 mrgGlobalLight_Diffuse;
//   float4 mrgGlobalLight_Direction;
//   float4 mrgGlobalLight_Specular;
//   float4 mrgInShadowScale;
//   float4 mrgLuminanceRange;
//   sampler2D sampDif;
//   sampler2D sampEnv;
//   sampler2D sampSpec;
//
//
// Registers:
//
//   Name                                  Reg   Size
//   ------------------------------------- ----- ----
//   mrgGlobalLight_Direction              c10      1
//   g_Diffuse                             c16      1
//   g_Ambient                             c17      1
//   g_Specular                            c18      1
//   g_PowerGlossLevel                     c20      1
//   g_OpacityReflectionRefractionSpectype c21      1
//   g_EyePosition                         c22      1
//   g_EyeDirection                        c23      1
//   mrgGlobalLight_Diffuse                c36      1
//   mrgGlobalLight_Specular               c37      1
//   mrgEyeLight_Diffuse                   c58      1
//   mrgEyeLight_Specular                  c59      1
//   mrgEyeLight_Range                     c60      1
//   mrgEyeLight_Attribute                 c61      1
//   mrgLuminanceRange                     c63      1
//   mrgInShadowScale                      c64      1
//   g_ShadowMapParams                     c65      1
//   g_GI0Scale                            c70      1
//   g_LightScatteringColor                c75      1
//   g_aLightField                         c77      6
//   g_ForceAlphaColor                     c89      1
//   g_SonicSkinFalloffParam               c150     1
//   g_ChrPlayableMenuParam                c152     1
//   sampDif                               s0       1
//   sampSpec                              s1       1
//   sampEnv                               s4       1
//   g_VerticalShadowMapSampler            s7       1
//   g_ShadowMapSampler                    s13      1
//

    ps_3_0
    def c0, 0.5, 0, 1, -2
    def c1, 0.0500000007, 10, -0.0500000007, 1.05263162
    def c2, -1, 1, 0, 0.5
    def c3, -2, 3, 500, 5
    def c4, 1024, 1, -1, 0
    def c5, 0.25, 4, 0, 0
    dcl_texcoord v0.xy
    dcl_texcoord2 v1.xyz
    dcl_texcoord3_pp v2
    dcl_texcoord4 v3
    dcl_texcoord6 v4
    dcl_texcoord5_pp v5.xyz
    dcl_color_pp v6
    dcl_2d s0
    dcl_2d s1
    dcl_2d s4
    dcl_2d s7
    dcl_2d s13
    max r0.x, v3.z, c0.y
    add r0.y, r0.x, -v3.w
    cmp r0.z, r0.y, c0.y, r0.x
    mov r0.xyw, v3
    texldp_pp r0, r0, s13
    max r0.y, v4.z, c0.y
    add r0.z, r0.y, -v4.w
    cmp r1.z, r0.z, c0.y, r0.y
    mov r1.xyw, v4
    texldp_pp r1, r1, s7
    min_pp r2.x, r1.x, r0.x
    lrp_pp r0.x, v5.z, c0.z, r1.x
    lrp_pp r0.y, v5.z, c0.z, r2.x
    rcp r0.z, v3.w
    mul r0.zw, r0.z, v3.xyxy
    mad r0.zw, r0, -c0.w, -c0.z
    abs_sat r0.zw, r0
    add_sat r0.zw, r0, -c65.x
    mul r0.zw, r0, c65.y
    max r1.x, r0.z, r0.w
    lrp_pp r2.x, r1.x, c0.z, r0.y
    lrp_pp r2.y, r1.x, c0.z, r0.x
    mul r0.x, r2.x, c77.w
    add_pp r0.y, -c61.z, c61.y
    rcp_pp r0.y, r0.y
    add r1.xyz, -c22, v1
    dp3 r0.z, r1, r1
    rsq r0.z, r0.z
    mul_pp r1.xyz, r0.z, r1
    rcp r0.z, r0.z
    add r0.z, r0.z, -c60.z
    dp3_pp r0.w, r1, c23
    add_pp r0.w, r0.w, -c61.z
    mul_sat_pp r0.y, r0.y, r0.w
    mad_pp r0.w, r0.y, c3.x, c3.y
    mul_pp r0.y, r0.y, r0.y
    mul_pp r0.y, r0.y, r0.w
    pow_sat_pp r1.w, r0.y, c61.w
    mov r2.z, c0.z
    add r0.y, r2.z, -c61.x
    cmp_pp r0.y, r0.y, c0.z, r1.w
    add r0.w, -c60.z, c60.w
    rcp r0.w, r0.w
    mul_sat r0.z, r0.w, r0.z
    add_pp r0.z, -r0.z, c0.z
    cmp_pp r0.z, -c61.x, r2.z, r0.z
    mul_pp r0.y, r0.y, r0.z
    mul_pp r2.xzw, c59.w, c59.xyyz
    mov_pp r0.zw, c3
    mul_pp r0.zw, r0, c20.xyyz
    mul_pp r2.xzw, r0.w, r2
    max_pp r1.w, r0.z, c0.z
    mul_pp r3.xyz, r0.w, c37
    min r0.z, r1.w, c4.x
    add_pp r4, c22.xyzx, -v1.xyzx
    dp3_pp r0.w, r4.yzww, r4.yzww
    rsq_pp r0.w, r0.w
    mad_pp r5.xyz, r4, r0.w, -r1
    nrm_pp r6.xyz, r5
    nrm_pp r5, v2.xyzx
    dp3_sat_pp r1.w, r6, r5
    pow r3.w, r1.w, r0.z
    mul_pp r2.xzw, r2, r3.w
    mul_pp r2.xzw, r0.y, r2
    mad_pp r6.xyz, r4, r0.w, -c10
    mul_pp r4, r0.w, r4
    nrm_pp r7.xyz, r6
    dp3_sat_pp r0.w, r7, r5
    pow r1.w, r0.w, r0.z
    mul_pp r3.xyz, r1.w, r3
    mad_pp r2.xzw, r3.xyyz, r0.x, r2
    mul_pp r2.xzw, r2, c18.xyyz
    add_pp r0.z, c152.x, -v1.y
    add_pp r0.z, r0.z, c1.x
    mul_sat_pp r0.z, r0.z, c1.y
    mul_pp r2.xzw, r0.z, r2
    mul r0.w, c64.z, c64.x
    mov r3.xz, c64
    mad r1.w, r3.x, -r3.z, r3.z
    mad_pp r0.w, r0.x, r1.w, r0.w
    dp3_pp r1.w, r4.yzww, r5.yzww
    add_pp r3.x, r1.w, r1.w
    add_sat_pp r1.w, -r1.w, c0.z
    pow_pp r3.y, r1.w, c150.z
    mul r1.w, r3.y, c150.y
    mad_pp r3, r3.x, r5, -r4
    mad_pp r4, r3.wyzw, c2.xxyz, c2.zzzw
    mul r6, r3, c4.yzzw
    cmp_pp r4, r3.z, r4, r6
    add_pp r3.x, r4.z, c0.z
    rcp r3.x, r3.x
    mad_pp r4.xy, r4.yxzw, r3.x, c0.z
    mad_pp r6.x, r4.y, c5.x, r4.w
    mul_pp r6.y, r4.x, c0.x
    texld_pp r4, r6, s4
    mul r6.xyz, r4, c63.x
    mad_pp r4.xyz, r6, r4.w, r4
    texld_pp r6, v0, s1
    mul_pp r4.xyz, r4, r6.w
    mul_pp r4.xyz, r0.w, r4
    mul_pp r4.xyz, r0.z, r4
    mad_pp r2.xzw, r2, r6.xyyz, r4.xyyz
    mul_pp r2.xzw, r2, v2.w
    dp3_pp r0.w, r5.wyzw, -r1
    add_pp r0.w, r0.w, c1.z
    mul_sat_pp r0.w, r0.w, c1.w
    mul r1.xyz, c58.w, c58
    mul_pp r1.xyz, r0.w, r1
    mul_pp r1.xyz, r0.y, r1
    dp3_pp r0.y, r5.wyzw, -c10
    add_pp r0.y, r0.y, c1.z
    mul_sat_pp r0.y, r0.y, c1.w
    mul_pp r4.xyz, r0.y, c36
    mad_pp r1.xyz, r4, r0.x, r1
    mad r0.x, r0.x, c0.x, c0.x
    mad_pp r0.x, r1.w, r0.x, c150.x
    mad_pp r4.xyz, r5.wyzw, c0.x, c0.x
    mul_pp r5.xyz, r5.wyzw, r5.wyzw
    mov r6.xyz, c79
    add r6.xyz, -r6, c80
    mad r7.xyz, r4.y, r6, c79
    mul r7.xyz, r5.y, r7
    mov r8.xyz, c77
    add r8.xyz, -r8, c78
    mad r4.xyw, r4.x, r8.xyzz, c77.xyzz
    mad_pp r4.xyw, r5.x, r4, r7.xyzz
    mov r7.xyz, c81
    add r5.xyw, -r7.xyzz, c82.xyzz
    mad r7.xyz, r4.z, r5.xyww, c81
    mad_pp r4.xyz, r5.z, r7, r4.xyww
    mul_pp r7.xyz, r4, c17
    mul r7.xyz, r7, c70.w
    mad r4.xyz, r4, c17, -r7
    mad_pp r4.xyz, r2.y, r4, r7
    add_pp r1.xyz, r1, r4
    mul_pp r1.xyz, r1, c16
    texld_pp r4, v0, s0
    lrp_pp r7, r0.z, r4, c0.z
    mad_pp r1.xyz, r1, r7, r2.xzww
    mul_pp r2.w, r7.w, c16.w
    mad_pp r4.xyz, r3.wyzw, c0.x, c0.x
    mul_pp r3.xyz, r3.wyzw, r3.wyzw
    mad r6.xyz, r4.y, r6, c79
    mul r6.xyz, r3.y, r6
    mad r4.xyw, r4.x, r8.xyzz, c77.xyzz
    mad r5.xyz, r4.z, r5.xyww, c81
    mad_pp r3.xyw, r3.x, r4, r6.xyzz
    mad_pp r3.xyz, r3.z, r5, r3.xyww
    mul_pp r0.yzw, r0.z, r3.xxyz
    mad_pp r2.xyz, r0.yzww, r0.x, r1
    mul_pp r0, r2, v6
    mul_pp r0.w, r0.w, c21.x
    mul_pp r0, r0, c89
    min_pp r1, r0, c5.y
    mul_pp r0.xyz, c75, v5.y
    mad_pp oC0.xyz, r1, v5.x, r0
    mov_pp oC0.w, r1.w

// approximately 171 instruction slots used (5 texture, 166 arithmetic)
