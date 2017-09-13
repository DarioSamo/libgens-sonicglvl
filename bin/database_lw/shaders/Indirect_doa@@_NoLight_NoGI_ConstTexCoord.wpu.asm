//
// Generated by Microsoft (R) HLSL Shader Compiler 9.29.952.3111
//
// Parameters:
//
//   float4 g_Ambient;
//   float4 g_Diffuse;
//   float4 g_OffsetParam;
//   float4 g_aLightField[6];
//   float4 mrgGlobalLight_Diffuse;
//   float4 mrgGlobalLight_Direction;
//   float4 mrgTexcoordIndex[4];
//   sampler2D sampDLScatter;
//   sampler2D sampDif;
//   sampler2D sampOffset0;
//   sampler2D sampOffsetMask;
//
//
// Registers:
//
//   Name                     Reg   Size
//   ------------------------ ----- ----
//   mrgGlobalLight_Direction c10      1
//   g_Diffuse                c16      1
//   g_Ambient                c17      1
//   mrgTexcoordIndex         c26      2
//   mrgGlobalLight_Diffuse   c36      1
//   g_aLightField            c77      6
//   g_OffsetParam            c150     1
//   sampDif                  s0       1
//   sampOffset0              s3       1
//   sampOffsetMask           s5       1
//   sampDLScatter            s9       1
//

    ps_3_0
    def c0, 0.5, 0, 0, 0
    def c1, 0, 1, 2, 3
    dcl_texcoord v0
    dcl_texcoord1 v1
    dcl_texcoord3 v2.xyz
    dcl_texcoord5 v3.xy
    dcl_color_pp v4
    dcl_2d s0
    dcl_2d s3
    dcl_2d s5
    dcl_2d s9
    frc r0.xy, c27.ywzw
    cmp r0.zw, -r0.xyxy, c1.x, c1.y
    add r0.xy, -r0, c27.ywzw
    mov r1.x, c1.x
    cmp r0.zw, c27.xyyw, r1.x, r0
    add r0.xy, r0.zwzw, r0
    add r2, r0.y, -c1
    add r0, r0.x, -c1
    cmp r1.yz, -r2_abs.x, v0.xxyw, c1.x
    cmp r1.yz, -r2_abs.y, v0.xzww, r1
    cmp r1.yz, -r2_abs.z, v1.xxyw, r1
    cmp r1.yz, -r2_abs.w, v1.xzww, r1
    texld r2, r1.yzzw, s5
    add_pp r1.y, r2.y, r2.y
    cmp r1.zw, -r0_abs.x, v0.xyxy, c1.x
    cmp r0.xy, -r0_abs.y, v0.zwzw, r1.zwzw
    cmp r0.xy, -r0_abs.z, v1, r0
    cmp r0.xy, -r0_abs.w, v1.zwzw, r0
    texld_pp r0, r0, s3
    mad r0.xy, r0.wyzw, r1.y, -c1.y
    frc r0.z, c26.x
    cmp r0.w, -r0.z, c1.x, c1.y
    add r0.z, -r0.z, c26.x
    cmp r0.w, c26.x, r1.x, r0.w
    add r0.z, r0.w, r0.z
    add r1, r0.z, -c1
    cmp r0.zw, -r1_abs.x, v0.xyxy, c1.x
    cmp r0.zw, -r1_abs.y, v0, r0
    cmp r0.zw, -r1_abs.z, v1.xyxy, r0
    cmp r0.zw, -r1_abs.w, v1, r0
    mad r0.xy, r0, c150, r0.zwzw
    texld_pp r0, r0, s0
    mul_pp r0.w, r0.w, c16.w
    mul_pp oC0.w, r0.w, v4.w
    nrm_pp r1.xyz, v2
    mad_pp r2.xyz, r1, c0.x, c0.x
    mov r3.xyz, c80
    mad r3.xyz, r3, r2.y, c79
    mul_pp r4.xyz, r1, r1
    dp3_sat_pp r0.w, r1, -c10
    mul r0.w, r0.w, c77.w
    mul_pp r1.xyz, r0.w, c36
    mul r3.xyz, r3, r4.y
    mov r5.xyz, c77
    mad r2.xyw, c78.xyzz, r2.x, r5.xyzz
    mov r5.xyz, c82
    mad r5.xyz, r5, r2.z, c81
    mad_pp r2.xyz, r4.x, r2.xyww, r3
    mad_pp r2.xyz, r4.z, r5, r2
    mad_pp r1.xyz, r2, c17, r1
    mul_pp r1.xyz, r1, c16
    mul_pp r0.xyz, r0, r1
    mul_pp r0.xyz, r0, v4
    texld_pp r1, v3.yxzw, s9
    mad_pp oC0.xyz, r0, r1.w, r1

// approximately 57 instruction slots used (4 texture, 53 arithmetic)