[declaration:"dfdfdf.eee"]
void MainPass(float r, float g, float b, out vec4 out_color)
{{
  void main()
  {
    out_color = vec4(r, g, b + 0.5f, 1.0f);
  }
}}

[include: "good_includes.glsl"]
void RenderGraphMain()
{{
  void main()
  {
    //Image test_img = GetImage(10, 20, rgba16f);
    //Image test_img = GetSwapchainImage();
    //Image mip_img = test_img.GetMip(3);
    //PrintImg(mip_img);
    //PrintImg(mip_img);
    MainPass(
      SliderFloat("R", 0.0f, 1.0f),
      SliderFloat("G", 0.0f, 1.0f),
      SliderFloat("B", 0.0f, 1.0f),
      GetSwapchainImage());
    int a = SliderInt("Int param", -42, 42, 5);
    float b = SliderFloat("Float param", -42.0f, 42.0f);
    //float e = SliderFloat("Float param", -42.0f, 42.0f);
    Text("script int: " + formatInt(a) + " float: " + formatFloat(b));
  }
}}

void NotMainPass(out vec4 also_color)
{{
  void main()
  {
    also_color = vec4(0.0f, 0.5f, 1.0f, 1.0f);
  }
}}

[ declaration : "kkk.bla" ]
[include : "dfdfdf.eee", "something.h"]
[numthreads (2, 6, 1)]
{{
  //void ColorPass(in vec2 mouse_pos, in float time, writeonly image2D<rgba32f> img, out vec4 color)

  void main()
  {
    color = vec4(sin(time));
    imageStore(img, ivec2(gl_FragCoord.xy), color * 0.5f);
  }
}}

[declaration: "render_graph_test.as"]
{{
  void RunTestGraph()
  {
    Image test_img = GetImage(GetSwapchainSize(), rgba16f);
    ColorPass(GetSwapchainSize(), GetMousePos(), test_img.GetMip(0));
    AnotherColorPass(test_img.GetMip(0), GetSwapchainImage());
  }
}}