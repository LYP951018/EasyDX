#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

struct Light
{
    float3 Position;
    float Padding1;
    float3 Direction;
    float Padding2;
    float4 Color;
    
    float SpotAngle;
    float3 Attenuation;

    int LightType;
    bool Enabled;
    float Range;
    float Padding;
};

struct Material
{
    float4 Amibient, Diffuse, Specular, Emissive;
    float SpecularPower;
    int UseTexture;
    float2 Padding;
};

//lightDir means LightPos - Pos
void ComputeDiffuse(Material material, Light light, float3 lightDir, float3 normal, out float4 diffuse)
{
    diffuse = 0.f;
    float diffuseFactor = dot(lightDir, normal);
    if (diffuseFactor > 0.0f)
    {
        diffuse = float4(diffuseFactor, diffuseFactor, diffuseFactor, 1.f) * light.Color * material.Diffuse;
    }
}

void ComputeAmbient(Material material, float4 globalAmbient, out float4 ambient)
{
    ambient = material.Amibient * globalAmbient;
}

void ComputeSpec(Material material, Light light, float3 lightDir, float3 viewDir, float3 normal, out float4 spec)
{
    float3 reflected = normalize(reflect(-lightDir, normal));
    float4 k = pow(max(dot(reflected, normalize(viewDir)), 0.f), material.SpecularPower);
    spec = k * material.Specular * light.Color;
}

void ComputeDirectionalLight(Material material,
                        Light light, float3 normal, float3 viewDir,
                        out float4 diffuse, out float4 spec)
{
    float3 lightDir = -light.Direction;
    ComputeDiffuse(material, light, lightDir, normal, diffuse);
    ComputeSpec(material, light, lightDir, viewDir, normal, spec);
}

float ComputeAttenuation(Light light, float distance)
{
    return dot(light.Attenuation, float3(1.f, distance, pow(distance, 2.f)));
}

void ComputePointLight(Material material,
                        Light light, float3 pos, float3 normal, float3 viewDir,
                        out float4 diffuse, out float4 spec)
{
    float3 direction = light.Position - pos;
    float distance = length(direction);
    if (distance > light.Range)
        return;
    direction = direction / distance;
    float attenuation = ComputeAttenuation(light, distance.x);
    ComputeDiffuse(material, light, direction, normal, diffuse);
    ComputeSpec(material, light, direction, viewDir, normal, spec);
    diffuse *= attenuation;
    spec *= attenuation;
    diffuse.w = 1.f;
    spec.w = 1.f;
}

void ComputeSpotLight(Material material,
                        Light light, float3 pos, float3 normal, float3 viewDir,
                        out float4 diffuse, out float4 spec)
{
    float3 direction = light.Position - pos;
    float distance = length(direction);
    if (distance > light.Range)
        return;
    direction = direction / distance;
    float spotFactor = max(cos(dot(-direction, light.Direction)), 0.f);
    float attenuation = ComputeAttenuation(light, distance.x);
    ComputeDiffuse(material, light, direction, direction, diffuse);
    ComputeSpec(material, light, direction, viewDir, normal, spec);
    diffuse = diffuse * attenuation * spotFactor;
    spec = spec * attenuation * spotFactor;
}

void ComputeLighting(float3 eyePos, float3 position, float3 normal, Light light,
    Material material, out float4 diffuse, out float4 spec)
{
    diffuse = 0.f;
    spec = 0.f;

    if (!light.Enabled)
        return;
    float3 viewDir = eyePos - position;
    switch (light.LightType)
    {
        case DIRECTIONAL_LIGHT:
            ComputeDirectionalLight(material, light, normal, viewDir, diffuse, spec);
            break;
        case POINT_LIGHT:
            ComputePointLight(material, light, position, normal, viewDir, diffuse, spec);
            break;
        case SPOT_LIGHT:
            ComputeSpotLight(material, light, position, normal, viewDir, diffuse, spec);
            break;
        default:
            break;
    }

    diffuse.w = 1.0f;
    spec.w = 1.0f;
}