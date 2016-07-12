// vertex shader in DepthShadowmap.hlsl
void casterVP(
   float4 position     : POSITION,
   out float4 outPos   : POSITION,
   out float2 outDepth : TEXCOORD0,

   uniform float4x4 worldViewProj,
   uniform float farPlane ) //Usá far_clip_distance para este
{
   outPos = mul(worldViewProj, position);
   outDepth.x = outPos.z / farPlane;
   outDepth.y = outPos.w;
}

// pixel shader in DepthShadowmap.hlsl
void casterFP(
   float2 depth      : TEXCOORD0,
   out float4 result : COLOR)
   
{
   depth.x=1.0;
   result = float4(depth.xxx, 1);
}