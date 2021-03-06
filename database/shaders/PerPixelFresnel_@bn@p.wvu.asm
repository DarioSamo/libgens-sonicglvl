//
// Generated by Microsoft (R) HLSL Shader Compiler 9.29.952.3111
//
// Parameters:
//
//   float4 g_EyePosition;
//   float4 g_LightScatteringFarNearScale;
//   float4 g_LightScattering_ConstG_FogDensity;
//   float4 g_LightScattering_Ray_Mie_Ray2_Mie2;
//   row_major float4x4 g_MtxLightViewProjection;
//   row_major float3x4 g_MtxPalette[25];
//   row_major float4x4 g_MtxProjection;
//   row_major float4x4 g_MtxVerticalLightViewProjection;
//   row_major float4x4 g_MtxView;
//   row_major float4x4 g_MtxWorld;
//   float4 g_VerticalLightDirection;
//   float4 mrgGIAtlasParam;
//   float4 mrgGlobalLight_Direction;
//   bool mrgHasBone;
//   float4 mrgMorphWeight;
//   float4 mrgTexcoordOffset[2];
//
//
// Registers:
//
//   Name                                Reg   Size
//   ----------------------------------- ----- ----
//   mrgHasBone                          b0       1
//   g_MtxProjection                     c0       4
//   g_MtxView                           c4       4
//   g_MtxWorld                          c8       4
//   g_MtxLightViewProjection            c24      4
//   g_MtxPalette                        c28     75
//   g_EyePosition                       c178     1
//   mrgGlobalLight_Direction            c183     1
//   mrgGIAtlasParam                     c186     1
//   mrgTexcoordOffset                   c191     2
//   mrgMorphWeight                      c194     1
//   g_LightScattering_Ray_Mie_Ray2_Mie2 c197     1
//   g_LightScattering_ConstG_FogDensity c198     1
//   g_LightScatteringFarNearScale       c199     1
//   g_MtxVerticalLightViewProjection    c207     4
//   g_VerticalLightDirection            c211     1
//

    vs_3_0
    def c12, 1, 3, 1.44269502, 1.5
    def c13, 0.5, -0.5, 0, 0
    dcl_position v0
    dcl_texcoord v1
    dcl_texcoord1 v2
    dcl_texcoord2 v3
    dcl_texcoord3 v4
    dcl_texcoord4 v5
    dcl_texcoord5 v6
    dcl_texcoord6 v7
    dcl_texcoord7 v8
    dcl_normal v9
    dcl_tangent v10
    dcl_binormal v11
    dcl_blendweight v12
    dcl_blendindices v13
    dcl_position o0
    dcl_texcoord o1
    dcl_texcoord1 o2
    dcl_texcoord2 o3
    dcl_texcoord3 o4
    dcl_texcoord4 o5
    dcl_texcoord6 o6
    dcl_texcoord5 o7
    dcl_texcoord7 o8
    dcl_texcoord8 o9
    mad r0.zw, v2.xyxy, c186.xyxy, c186
    mov r0.xy, v1
    add o1, r0, c191
    mov r0.xy, v3
    mov r0.zw, v4.xyxy
    add o2, r0, c192
    mul r0.xyz, c194.y, v6
    mad r0.xyz, c194.x, v5, r0
    mad r0.xyz, c194.z, v7, r0
    mad r0.xyz, c194.w, v8, r0
    add r0.xyz, r0, v0
    if b0
      add r1.x, c12.x, -v12.x
      add r1.x, r1.x, -v12.y
      add r1.x, r1.x, -v12.z
      mul r2, c12.y, v13
      mova a0, r2.yxzw
      mul r3, v12.y, c30[a0.x]
      mad r3, c30[a0.y], v12.x, r3
      mad r3, c30[a0.z], v12.z, r3
      mad r2, c30[a0.w], r1.x, r3
      dp3 r1.y, r2, v9
      mov r0.w, v0.w
      dp4 r1.z, r2, r0
      mul r3, v12.y, c28[a0.x]
      mad r3, c28[a0.y], v12.x, r3
      mad r3, c28[a0.z], v12.z, r3
      mad r3, c28[a0.w], r1.x, r3
      dp3 r1.w, r3, v9
      dp4 r2.w, r3, r0
      dp3 r3.w, r3, v10
      mul r4, v12.y, c29[a0.x]
      mad r4, c29[a0.y], v12.x, r4
      mad r4, c29[a0.z], v12.z, r4
      mad r4, c29[a0.w], r1.x, r4
      dp4 r0.y, r4, r0
      dp3 r0.w, r4, v9
      dp3 r1.x, r4, v10
      dp3 r4.w, r2, v10
      dp3 r3.x, r3, -v11
      dp3 r3.y, r4, -v11
      dp3 r2.x, r2, -v11
      mov r0.x, r2.w
      mov r0.z, r1.z
      mov r1.z, v0.w
    else
      mov r3.w, v10.x
      mov r1.x, v10.y
      mov r4.w, v10.z
      mov r3.xy, -v11
      mov r2.x, -v11.z
      mov r0.w, v9.y
      mov r1.yw, v9.xzzx
      mov r1.z, v0.w
    endif
    mul r2.yzw, r1.x, c9.xxyz
    mad r2.yzw, r3.w, c8.xxyz, r2
    mad r2.yzw, r4.w, c10.xxyz, r2
    dp3 r1.x, r2.yzww, r2.yzww
    rsq r1.x, r1.x
    mul o8.xyz, r1.x, r2.yzww
    mul r2.yzw, r3.y, c9.xxyz
    mad r2.yzw, r3.x, c8.xxyz, r2
    mad r2.xyz, r2.x, c10, r2.yzww
    dp3 r1.x, r2, r2
    rsq r1.x, r1.x
    mul o9.xyz, r1.x, r2
    mul r2, r0.y, c9
    mad r2, r0.x, c8, r2
    mad r2, r0.z, c10, r2
    mad r2, r1.z, c11, r2
    mul r3, r2.y, c5
    mad r3, r2.x, c4, r3
    mad r3, r2.z, c6, r3
    mad r3, r2.w, c7, r3
    mul r4, r3.y, c1
    mad r4, r3.x, c0, r4
    mad r4, r3.z, c2, r4
    mad o0, r3.w, c3, r4
    mov o3.xyz, r2
    mul r0.xyz, r0.w, c9
    mad r0.xyz, r1.w, c8, r0
    mad r0.xyz, r1.y, c10, r0
    nrm r1.xyz, r0
    mov o4.xyz, r1
    add r0.xyz, -r2, c178
    nrm r4.xyz, r0
    dp3_sat r0.x, r1, r4
    add o7.w, -r0.x, c12.x
    mov o3.w, -r3.z
    dp3 r0.x, -c183, r4
    mad r0.y, c198.z, r0.x, c198.y
    pow r2.w, r0_abs.y, c12.w
    rcp r0.y, r2.w
    mul r0.y, r0.y, c198.x
    mul r0.y, r0.y, c197.w
    mad r0.x, r0.x, r0.x, c12.x
    mad r0.x, c197.z, r0.x, r0.y
    add r0.y, c197.y, c197.x
    rcp r0.z, r0.y
    mul r0.x, r0.z, r0.x
    add r0.z, -r3.z, -c199.y
    mul_sat r0.z, r0.z, c199.x
    mul r0.z, r0.z, c199.z
    mul r0.y, r0.z, -r0.y
    mul r0.y, r0.y, c12.z
    exp r0.y, r0.y
    mov o7.x, r0.y
    add r0.y, -r0.y, c12.x
    mul r0.x, r0.y, r0.x
    mul o7.y, r0.x, c199.w
    mov r1.w, c12.x
    dp4_sat r0.x, r1, c211
    add o7.z, -r0.x, c12.x
    mov o4.w, c13.z
    mul r0, r2.y, c25
    mad r0, r2.x, c24, r0
    mad r0, r2.z, c26, r0
    add r0, r0, c27
    mov o5.zw, r0
    mul r0.xyz, r0.xyww, c13.xyxw
    add o5.xy, r0.z, r0
    mul r0, r2.y, c208
    mad r0, r2.x, c207, r0
    mad r0, r2.z, c209, r0
    add r0, r0, c210
    mov o6.zw, r0
    mul r0.xyz, r0.xyww, c13.xyxw
    add o6.xy, r0.z, r0
    mov o8.w, c13.z
    mov o9.w, c13.z

// approximately 139 instruction slots used
