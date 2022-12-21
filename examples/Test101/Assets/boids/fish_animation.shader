#if 0

// references:
// https://github.com/albertomelladoc/Fish-Animation/blob/master/FishAnimation.shader
// https://docs.godotengine.org/en/stable/tutorials/performance/vertex_animation/animating_thousands_of_fish.html

Shader {
	Properties {
		Float	test  = 0.5
		Vec4f	test2 = {0,0,0,1}
		
		[DisplayName="Color Test"]
		Color4f	color = {1,1,1,1}
		
		Texture2D mainTex
	}
	
	Pass {
		// Queue	"Transparent"
		//Cull		None

		DepthTest	LessEqual

//		DepthTest	Always
//		DepthWrite	false

		BlendRGB 	Add One OneMinusSrcAlpha
		BlendAlpha	Add One OneMinusSrcAlpha
		
		VsFunc		vs_main
		PsFunc		ps_main
	}
}
#endif

struct VertexIn {
	float4 positionOS : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL;
};

struct PixelIn {
	float4 positionHCS : SV_POSITION;
	float4 positionWS  : TEXCOORD10;
	float2 uv		: TEXCOORD0;
	float4 color  	: COLOR;
	float3 normal 	: NORMAL;
};

float4x4	sge_matrix_model;
float4x4	sge_matrix_view;
float4x4	sge_matrix_proj;
float4x4	sge_matrix_mvp;

float3		sge_camera_pos;
float3		sge_light_pos;
float3		sge_light_dir;
float		sge_light_power;
float3		sge_light_color;

float4		sge_time;

float  test_float;
float4 test_color;

Texture2D mainTex;
SamplerState mainTex_Sampler;

float _time_scale;
float  _speed;
float3 _frequency;
float3 _amplitude;
float  _headLimit;

#define IMPL2 0

#if	IMPL2
float _side_to_side;
float _pivot;
float _wave;
float _twist;
float _mask_black;
float _mask_white;
#endif

PixelIn vs_main(VertexIn i) {
	PixelIn o;

	float time 		= sge_time.x * _time_scale;

	i.positionOS.z += sin((i.positionOS.z + time * _speed) * _frequency.x) * _amplitude.x;		
	i.positionOS.y += sin((i.positionOS.z + time * _speed) * _frequency.y) * _amplitude.y;
	if (i.positionOS.z > _headLimit)
	{
		i.positionOS.x += sin((0.05 + time * _speed) * _frequency.z)* _amplitude.z * _headLimit;
	}
	else
	{
		i.positionOS.x += sin((i.positionOS.z + time * _speed) * _frequency.z) * _amplitude.z * i.positionOS.z;
	}

#if IMPL2

	i.positionOS.x += cos(time) * _side_to_side;

	//angle is scaled by 0.1 so that the fish only pivots and doesn't rotate all the way around
	//pivot is a uniform float
	float 	 pivot_angle 	 = cos(time) * 0.1 * _pivot;
	float2x2 rotation_matrix = float2x2(float2(cos(pivot_angle), -sin(pivot_angle)), float2(sin(pivot_angle), cos(pivot_angle)));
	i.positionOS.xz 		 = mul(rotation_matrix, i.positionOS.xz);

	float body = (i.positionOS.z + 1.0) / 2.0;
	i.positionOS.x += cos(time + body) * _wave;

	float twist_angle 		= cos(time + body) * 0.3 * _twist;
	float2x2 twist_matrix 	= float2x2(float2(cos(twist_angle), -sin(twist_angle)), float2(sin(twist_angle), cos(twist_angle)));
	i.positionOS.xy 		= mul(twist_matrix, i.positionOS.xy);
	
	float mask 		= smoothstep(_mask_black, _mask_white, 1.0 - body);
	i.positionOS.x += cos(time + body) * mask * _wave;
	i.positionOS.xy = lerp(i.positionOS.xy, mul(twist_matrix, i.positionOS.xy), mask);

#endif

	o.positionWS  = mul(sge_matrix_model, i.positionOS);
	o.positionHCS = mul(sge_matrix_mvp,   i.positionOS);
	
	o.uv     = i.uv;
	o.color	 = i.color;
	o.normal = i.normal;
	return o;
}

struct Surface {
	float3 positionWS;
	float3 normal;	
	float3 color;
	float3 ambient;
	float3 diffuse;
	float3 specular;
	float  shininess;
};

float3 Color_Linear_to_sRGB(float3 x) { return x < 0.0031308 ? 12.92 * x : 1.13005 * sqrt(x - 0.00228) - 0.13448 * x + 0.005719; }
float3 Color_sRGB_to_Linear(float3 x) { return x < 0.04045 ? x / 12.92 : -7.43605 * x - 31.24297 * sqrt(-0.53792 * x + 1.279924) + 35.34864; }

float3 lighting_blinn_phong(Surface s) {
	float3 normal   = normalize(s.normal);
	float3 lightDir = s.positionWS - sge_light_pos;
	float  lightDistance = length(sge_light_dir);
	lightDir = normalize(lightDir);

	float lambertian = max(dot(normal, -sge_light_dir), 0);
	float specularPower = 0;

	if (lambertian > 0) {
		float3 viewDir = normalize(sge_camera_pos - s.positionWS);

		if (1) { // blinn-phong
			float3 halfDir = normalize(viewDir - lightDir);
			float specAngle = saturate(dot(halfDir, normal));
			specularPower = pow(specAngle, s.shininess);

		} else { // phong
			float3 reflectDir = reflect(-lightDir, normal);
			float specAngle = max(dot(reflectDir, viewDir), 0);
			specularPower = pow(specAngle, s.shininess / 4.0);
		}
	}

	float3 outLightColor = sge_light_color * sge_light_power / (lightDistance * lightDistance);

	float3 outAmbient  = s.ambient;
	float3 outDiffuse  = s.diffuse  * lambertian    * outLightColor;
	float3 outSpecular = s.specular * specularPower * outLightColor;

	float3 outColor = (outAmbient + outDiffuse + outSpecular) * s.color;
	return outColor;
}

float4 ps_main(PixelIn i) : SV_TARGET
{
//	return float4(i.positionHCS.w * 0.05, 0, 0, 1);
//	return float4(i.normal, 1);
//	return i.color * test_color;
//	return float4(i.uv.x, i.uv.y, 0, 1);

	Surface s;
	s.positionWS = i.positionWS;
	s.normal     = i.normal;
	s.color	     = test_color;
	s.specular   = float3(0.8, 0.8, 0.8);
	s.ambient    = float3(0.2, 0.2, 0.2);
	s.diffuse	 = float3(1, 1, 1);
	s.shininess	 = 1;
	
	float2 uv = i.uv;
	uv.x =  i.uv.x;
	uv.y = -i.uv.y;

	float4 texCol = mainTex.Sample(mainTex_Sampler, uv);
	
	float3 color = lighting_blinn_phong(s) * texCol;
	return float4(Color_Linear_to_sRGB(color), 1);
}
