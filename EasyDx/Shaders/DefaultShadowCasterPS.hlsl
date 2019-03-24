float4 main(float4 position: SV_POSITION) : SV_TARGET
{
	float d = position.z / position.w;
	return float4(d, d, d, d);
}