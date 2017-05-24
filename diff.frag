uniform sampler2D render;
uniform sampler2D orig;
 
void main()
{
    vec4 r = texture2D(render, gl_TexCoord[0].st);;
	vec4 o = texture2D(orig, gl_TexCoord[0].st);
	vec4 diff = (r - o);
	gl_FragColor.r = dot(diff, diff);
	gl_FragColor.g = 0.0;
	gl_FragColor.b = 0.0;
	gl_FragColor.a = 1.0;
}