uniform sampler2DRect externalTexture;
void main (void)
{

	gl_FragData[0] = texture2DRect(externalTexture, gl_TexCoord[0].st);
}

//uniform sampler2DRect bumpTex;
//uniform sampler2DRect reflectTex;
//
//uniform vec3 sunDirection;
//uniform float sunStrength;
//
//uniform float bumpWidth;
//uniform float bumpHeight;
//uniform float bumpBias;
//
//uniform vec2 bumpOffset; //increases over time
//uniform vec2 bumpWobble; //2 channel sinusoidal
//
//uniform float reflectWidth;
//uniform float reflectHeight;
//uniform float reflectDistortion;
//
//uniform float brightness;
//uniform float contrast;
//
//varying vec2 bumpTexCoord;
//varying vec2 reflectTexCoord;
//
//varying vec3 normal;
//varying vec3 tangent3d;
//
//varying vec3 viewDir;
//varying vec3 lightDir;
//varying float centerDistance;
//
//void main (void)
//{
//	vec3 result;
//	
//    vec4 buv = mod(vec4((bumpTexCoord.x + bumpOffset.x)+bumpBias, (bumpTexCoord.y + bumpWobble.x)+bumpBias, 
//                        bumpTexCoord.x + bumpOffset.y, bumpTexCoord.y + bumpWobble.y),
//                   vec4(bumpWidth, bumpHeight, bumpWidth, bumpHeight));
//
//    vec3 tangentNormal0 = (texture2DRect(bumpTex, buv.xy).rgb * 2.0) - 1.0;
//	vec3 tangentNormal1 = (texture2DRect(bumpTex, buv.zw).rgb * 2.0) - 1.0;
//	vec3 tangentNormal = normalize(tangentNormal0 * tangentNormal1);
//	
////	vec4 bumpSample = 2.0 * texture2DRect(bumpTex, bumpTexCoord.xy) - 1.0,  );
//	float sunMultiplier =  max(1.0 - centerDistance/(1000.0 * pow(sunStrength, 1.0)), 0.0);
//		
//	float wavefalloff = ( 1.0 - dot( viewDir, normal) ) * sunMultiplier;
//	wavefalloff = clamp( (wavefalloff - 0.5) * contrast + 0.5 + brightness, 0.0, 1.0);
//	
//	//float specular = pow( dot(tangent3d, tangentNormal), 2.0/wavefalloff);
//	float specular = dot(tangent3d, tangentNormal) * wavefalloff;
//	
//	vec2 reflectDimensions = vec2(reflectWidth, reflectHeight);
//	vec2 reflectSample = mod(reflectTexCoord.xy + tangentNormal.xy * reflectDimensions*reflectDistortion, reflectDimensions); 
//	//vec2 reflectSample = mod(reflectTexCoord.xy, reflectDimensions); 
//	
//	vec3 oceanblue = vec3( 0.0, 72.0/255.0, 1.0);
//	vec3 reflect = texture2DRect(reflectTex, reflectSample).rgb * oceanblue;
//	result = reflect * wavefalloff;
//	
//	//result = vec3( wavefalloff );	
//	//result = vec3(specular);
//	
//	gl_FragData[0].rgb = result;
//
///*
//    vec3 normViewDir = normalize(viewDir);
//    
//    //half4 buv = half4(i.bumpTexCoord.x + _Time.x * .003, i.bumpTexCoord.y + _SinTime.x * 0.0002, i.bumpTexCoord.x + _Time.y * 0.00004, i.bumpTexCoord.y + _SinTime.y * 0.005);
//    vec4 buv = mod(vec4((bumpTexCoord.x + bumpOffset.x)/bumpBias, (bumpTexCoord.y + bumpWobble.x)/bumpBias, 
//                        bumpTexCoord.x + bumpOffset.y, bumpTexCoord.y + bumpWobble.y),
//                   vec4(bumpWidth, bumpHeight, bumpWidth, bumpHeight));
//    
//    vec3 tangentNormal0 = (texture2DRect(bumpTex, buv.xy).rgb * 2.0) - 1.0;
//	vec3 tangentNormal1 = (texture2DRect(bumpTex, buv.zw).rgb * 2.0) - 1.0;
//	vec3 tangentNormal = normalize(tangentNormal0 + tangentNormal1);
//    
//    vec3 halfVec = normalize(normViewDir - normalize(lightDir));
//    float specular = pow(max(dot(halfVec, tangentNormal), 0.0), 250.0 * 1.0/sunStrength);
//    
//	vec2 reflectDimensions = vec2(reflectWidth, reflectHeight);
//	vec2 reflectSample = mod(reflectTexCoord.xy + tangentNormal.xy * reflectDimensions*.5, reflectDimensions); 
//	//gl_FragData[0].rgb = texture2DRect(reflectTex, reflectSample).rgb * tangentColor.w;
//*/
//	
//	//gl_FragData[0].rgb = tangentColor.xyz + vec3(.5);
//	//gl_FragData[0].rgb = texture2DRect(reflectTex, reflectTexCoord.xy ).rgb;
//	
//    //gl_FragData[0].rgb = vec3(specular);
//    //gl_FragData[0].rgb = vec3(1., 1., 0.);
//    //gl_FragData[0].rgb = vec3(dot(halfVec, tangentNormal) * 1.0) * texture2DRect(reflectTex, reflectTexCoord.xy ).rgb;
//    //gl_FragData[0].rgb = halfVec;
//    //gl_FragData[0].rgb = tangentNormal;
//    //gl_FragData[0].rgb = lightDir;
//    //gl_FragData[0].a = 1.0;
//    //gl_FragData[0] *= gl_Color;
//    //gl_FragData[0] = vec4( lightDir.xyz* vec3(.5) + vec3(.5), 1.0);
//
//}
