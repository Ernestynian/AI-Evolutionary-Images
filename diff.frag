uniform sampler2D render;
uniform sampler2D orig;
 
void main()
{
    vec4 r = texture2D(render, gl_TexCoord[0].st);
	vec4 o = texture2D(orig, gl_TexCoord[0].st);
	vec4 diff = (r - o);
	//gl_FragData[0].r = diff.r * diff.r + diff.g * diff.g + diff.b * diff.b;
	gl_FragData[0].r = dot(diff, diff);
	gl_FragData[0].g = 0.0;
	gl_FragData[0].b = 0.0;
	gl_FragData[0].a = 1.0;
}