uniform sampler2D tex;
 
void main()
{
    /*vec4 render = gl_Color;
	vec4 orig = texture2D(tex, gl_TexCoord[0].st);
	vec4 diff = (render - orig);
	gl_FragColor.r = dot(diff, diff);
	gl_FragColor.g = 0.0;
	gl_FragColor.b = 0.0;
	gl_FragColor.a = 1.0;*/
	gl_FragColor = texture2D(tex, gl_TexCoord[0].st);
}