
void main(void)
{
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
}

//uniform sampler2DRect bumpTex;
//uniform sampler2DRect reflectTex;
//
////uniform vec3 sunDirection;
//uniform float sunStrength;
//uniform vec3 centerPosition;
//varying float centerDistance;
//
//uniform float bumpScale;
//uniform float bumpSquish;
//uniform float bumpBias;
//
//uniform float reflectWidth;
//uniform float reflectHeight;
//uniform float reflectScale;
//uniform float reflectSquish;
//uniform vec2 reflectOffset;
//
//varying vec2 bumpTexCoord;
//varying vec2 reflectTexCoord;
//
//varying vec3 viewDir;
//varying vec3 lightDir;
//
//varying vec3 tangent3d;
//varying vec3 normal;
//
//attribute vec4 tangent;
//
//void main(void)
//{
//    bumpTexCoord.xy = gl_Vertex.xz * vec2(bumpScale*bumpSquish, bumpScale);
//	reflectTexCoord.xy = (gl_Vertex.xz + reflectOffset) * vec2(reflectScale*reflectSquish, reflectScale);
//	
//    normal = gl_Normal;
//	vec3 vecToLight = gl_Vertex.xyz - centerPosition;
//	viewDir = normalize( vecToLight );
//    centerDistance = length(vecToLight);
//	
//	tangent3d = tangent.xyz;
//	
//    gl_FrontColor = gl_Color;
//	gl_Position = ftransform();
//	
////    vec4 projSource = vec4(gl_Vertex.x, 0.0, gl_Vertex.z, 1.0);
////    vec4 projectedSource = gl_ModelViewMatrix, projSource;
//	
//    //calcualte light direction
////    vec3 binormal = cross( normal, tangent.xyz);
////    mat3 rotation = mat3( tangent.xyz, binormal, normal);
////    lightDir = rotation * sunDirection;
//	
//    //lightDir = sunDirection;
//    //calculate view direction
////    viewDir = rotation * normalize( vec4(centerPosition,0) - gl_Vertex) ).xyz;
//	//viewDir = rotation * normalize(gl_ModelViewMatrix * gl_Vertex ).xyz;
//
//	
////	gl_TexCoord[0] = gl_MultiTexCoord0;
////    gl_TexCoord[1] = gl_MultiTexCoord1; 
//}
