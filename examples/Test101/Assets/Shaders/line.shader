#if 0
Shader {
	Properties {
	}
	
	Pass {
		// Queue	"Transparent"
//		Cull		None

//		DepthTest	LessEqual

		DepthTest	Always
//		DepthWrite	false

		BlendRGB 	Add One OneMinusSrcAlpha
		BlendAlpha	Add One OneMinusSrcAlpha
		
		VsFunc		vs_main
		PsFunc		ps_main
	}

	Permutation
	{
		ENABLE_FEATURE_X 	= {1, 0, }
		//
		FEATURE_X_VAL 		=         {  15, 0.02f, }
		FEATURE_X_VAL2 		= {"asdsad"    ,            "ASdsa", asdsdsa_Asd, true}
		ENABLE_feature_y	= {false}
		ENABLE_feature		= {0, 1}

	}
}
#endif

struct VertexIn {
	float4 positionOS : POSITION;
	float4 color : COLOR;
};

struct PixelIn {
	float4 positionHCS : SV_POSITION;
	float4 color : COLOR;
};

float4x4	sge_matrix_mvp;

PixelIn vs_main(VertexIn i) {
	PixelIn o;
	o.positionHCS = mul(sge_matrix_mvp,   i.positionOS);
	o.color	 = i.color;
	return o;
}

float4 ps_main(PixelIn i) : SV_TARGET {
	//float4 color = {1, 1, 1, 1}; 
	//return color;
	#if	ENABLE_feature
	return float4(0, 1, 1, 1);
	#else
	return i.color;
	#endif
}
