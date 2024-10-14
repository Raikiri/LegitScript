void ColorPass(in float r, in float g, in float b, out vec4 out_color)
{{
  void main()
  {
    out_color = vec4(r, g, b + 0.5f, 1.0f);
  }
}}

void RenderGraphMain()
{{
  void main()
  {
    vec3 v1 = vec3(1, 3, 0);
    vec3 v2 = vec3(1, 0, 3.1);
    ((v1 + v2) * 3).Print();
    
    Image img = GetImage(128, 128, rgba8);
    Text("Mips count: " + img.GetMipsCount() + ". Mip 2 size: " + img.GetMip(0).GetSize());
    
    //Image img = GetImage(128, 128, rgba8);
    //ColorPass(
    //  SliderFloat("R", 0.0f, 1.0f) + 0.5f,
    //  SliderFloat("G", 0.0f, 1.0f),
    //  SliderFloat("B", 0.0f, 1.0f),
    //  GetSwapchainImage());
    //int a = SliderInt("Int param", -42, 42, 5);
    //float b = SliderFloat("Float param", -42.0f, 42.0f);
    //float e = SliderFloat("Float param", -42.0f, 42.0f);
    //Text("script int: " + formatInt(a) + " float: " + formatFloat(b));
  }
}}