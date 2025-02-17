#include "ModuleEditor.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include <GL/gl.h>
#include "ImGui/backends/imgui_impl_opengl3.h"
#include "ImGui/backends/imgui_impl_sdl2.h"

#include "parson.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModuleHierarchy.h"
#include "OurPrimitive.h"
#include "ModuleInput.h"
#include "ComponentMesh.h"
#include "ComponentTransform.h"

#include "Assimp/include/ai_assert.h"
#include "Assimp/include/version.h"

ModuleEditor::ModuleEditor(Application* app, bool start_enabled) : Module(app, start_enabled)
{
    logs = new std::deque<std::string>();
}

ModuleEditor::~ModuleEditor()
{
}

bool ModuleEditor::Init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(App->window->window, App->renderer3D->context);
    ImGui_ImplOpenGL3_Init();

    mFPSLog.reserve(30);
    mMsLog.reserve(30);

    gameWindowSize = { 200,200 };
    sceneWindowSize = { 200,200 };

    //Editor attributes Initialization
    showConsoleWindow = false;
    showAboutWindow = false;
    wireframeMode = false;

    //Window info
    fullscreen = App->window->IsFullscreen();
    resizable = App->window->IsResizable();
    borderless = App->window->IsBorderless();
    fulldesktop = App->window->IsFulldesktop();
    brightness = SDL_GetWindowBrightness(App->window->window);
    
    //Renderer info
    vsync = App->renderer3D->GetVsync();
    depthTest = App->renderer3D->GetDepthTestAttribute();
    cullFace = App->renderer3D->GetCullFaceAttribute();
    lighting = App->renderer3D->GetLightingAttribute();
    coloMaterial = App->renderer3D->GetColorMaterialAttribute();
    texture2D = App->renderer3D->GetTexture2DAttribute();
    blend = App->renderer3D->GetBlendAttribute();
    alphaTest = App->renderer3D->GetAlphaTestAttribute();
    lineSmooth = App->renderer3D->GetLineSmoothAttribute();
    pointSmooth = App->renderer3D->GetPointSmoothAttribute();
    polygonSmooth = App->renderer3D->GetPolygonSmoothAttribute();

    //Hardware Info
    SDL_version versionSDL;
    SDL_GetVersion(&versionSDL);
   
    sdlVersion = std::string("SDL Version: ") +
                 std::to_string(versionSDL.major) + "." +
                 std::to_string(versionSDL.minor) + "." +
                 std::to_string(versionSDL.patch);

    assimpVersion = std::to_string(aiGetVersionMajor()) + '.' + std::to_string(aiGetVersionMinor()) + '.' + std::to_string(aiGetVersionRevision());

    systemRAM = (float) SDL_GetSystemRAM() / (1024.f);
    cpuCount = SDL_GetCPUCount();
    cpuCacheSize = SDL_GetCPUCacheLineSize();

    int numScreens = SDL_GetNumVideoDisplays();
    if (numScreens > 0)
    {
        for (int i = 0; i < numScreens; ++i) {
            SDL_DisplayMode displayMode;
            if (SDL_GetCurrentDisplayMode(i, &displayMode) == 0) {
                screens.push_back(SDL_GetDisplayName(i));
            }
        }
    }

    Now3D = SDL_Has3DNow();
    AltiVec = SDL_HasAltiVec();
    AVX = SDL_HasAVX();
    AVX2 = SDL_HasAVX2();
    MMX = SDL_HasMMX();
    RDTSC = SDL_HasRDTSC();
    SSE = SDL_HasSSE();
    SSE2 = SDL_HasSSE2();
    SSE3 = SDL_HasSSE3();
    SSE41 = SDL_HasSSE41();
    SSE42 = SDL_HasSSE42();

    //About Window
    license = ReadFileIO("../../License.txt");

    isActiveHierarchy = true;
    isActiveConsole = true;
    isActiveInspector = true;
    isActiveSceneWindow = true;
    isActiveGameWindow = true;

    return true;
}

bool ModuleEditor::CleanUp()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    RELEASE(logs);


    return true;
}

update_status ModuleEditor::DrawEditor()
{
    update_status ret = update_status::UPDATE_CONTINUE;
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoNavFocus;

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(main_viewport->WorkPos);
    ImGui::SetNextWindowSize(main_viewport->Size);
    ImGui::SetNextWindowViewport(main_viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockingInv", nullptr, flags);

    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::End();

    UpdatePlots();

    ImGuiIO& io = ImGui::GetIO();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Quit Application", "ESC")) {
                ret = UPDATE_STOP;
            }
           
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Game Objects"))
        {

            if (ImGui::MenuItem(" EMPTY  ")) {

                PrimitivesGeomtriesLibrary::InstanciatePrimitiveGeometry(GeometryType::EMPTY);
            }

            if (ImGui::MenuItem(" CUBE  ")) {
                
                PrimitivesGeomtriesLibrary::InstanciatePrimitiveGeometry(GeometryType::P_CUBE);
            }

            if (ImGui::MenuItem(" PLANE "))
            {
                PrimitivesGeomtriesLibrary::InstanciatePrimitiveGeometry(GeometryType::P_PLANE);
            }

            if (ImGui::MenuItem(" SPHERE  "))
            {
                PrimitivesGeomtriesLibrary::InstanciatePrimitiveGeometry(GeometryType::P_SPHERE);
            }

            if (ImGui::MenuItem(" ICOSPHERE  "))
            {
                PrimitivesGeomtriesLibrary::InstanciatePrimitiveGeometry(GeometryType::P_ICOSHPERE);
            }

            if (ImGui::MenuItem(" CONE  "))
            {
                PrimitivesGeomtriesLibrary::InstanciatePrimitiveGeometry(GeometryType::P_CONE);
            }

            if (ImGui::MenuItem(" CYLINDER  "))
            {
                PrimitivesGeomtriesLibrary::InstanciatePrimitiveGeometry(GeometryType::P_CYLINDER);
            }

            if (ImGui::MenuItem(" TORUS  "))
            {
                PrimitivesGeomtriesLibrary::InstanciatePrimitiveGeometry(GeometryType::P_TORUS);
            }

            if (ImGui::MenuItem(" SUZANNE  "))
            {
                PrimitivesGeomtriesLibrary::InstanciatePrimitiveGeometry(GeometryType::P_SUZANNE);
            }


            ImGui::EndMenu();
        }
     

        if (ImGui::BeginMenu("Configuration"))
        {
            if (ImGui::Checkbox("Vsync", &vsync))
            {
                App->renderer3D->SetVsync(vsync);
            }

            std::string fps = std::to_string(AverageValueFloatVector(mFPSLog));
            std::string fpsText = "Average FPS: " + fps;
            ImGui::Text(fpsText.c_str());
            ImGui::PlotHistogram("FPS", &mFPSLog[0], mFPSLog.size(), 0, "FPS", 0.0f, 100.0f, ImVec2(450, 100));


            std::string ms = std::to_string(AverageValueFloatVector(mMsLog));
            std::string msText = "Average Miliseconds: " + ms;
            ImGui::Text(msText.c_str());
            ImGui::PlotHistogram("Ms.", &mMsLog[0], mMsLog.size(), 0, "Miliseconds", 0.0f, 100.0f, ImVec2(450, 100));


            WindowCollapsingHeader();

            RenderCollapsingHeader();
            
            if (ImGui::CollapsingHeader("Inputs"))
            {
                if (ImGui::IsMousePosValid())
                    ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
                else
                    ImGui::Text("Mouse pos: <INVALID>");

                ImGui::Text("Mouse down:");
                for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseDown(i)) { ImGui::SameLine(); ImGui::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]); }
                ImGui::Text("Mouse wheel: %.1f", io.MouseWheel);
                
                ImGuiKey start_key = (ImGuiKey)0;
                struct funcs { static bool IsLegacyNativeDupe(ImGuiKey key) { return key < 512 && ImGui::GetIO().KeyMap[key] != -1; } }; 
                ImGui::Text("Keys down:");         for (ImGuiKey key = start_key; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) { if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyDown(key)) continue; ImGui::SameLine(); ImGui::Text((key < ImGuiKey_NamedKey_BEGIN) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key); }

                
            }

            HardwareCollapsingHeader();

            if (ImGui::Button("Close", ImVec2(60, 0)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About..."))
            {
                showAboutWindow = true; 
            }

            ImGui::EndMenu();
        }


        //CreateAboutModalPopup(showModalAbout);
        CreateAboutWindow(showAboutWindow);
        CreateConsoleWindow(isActiveConsole);
        GameWindow(isActiveGameWindow);
        SceneWindow(isActiveSceneWindow);

        ViewCollapsingHeader();

        ImGui::EndMainMenuBar();
    }

    GameStateButtons();

    if (App->hierarchy->objSelected && isActiveInspector) {

        if (ImGui::Begin("Inspector")) {
            
            App->hierarchy->objSelected->PrintInspector();
        }
        ImGui::End();
    }

  

    if (isActiveHierarchy) {
        if (ImGui::Begin("GameObjects Hierarchy")) {

            App->hierarchy->GameObjectTree(App->scene->root, 0);


        }
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return ret;
}

void ModuleEditor::GameStateButtons()
{
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;

    float windowWidth = 105.0f;
    float windowHeight = 40.0f;

    float windowPosX = (ImGui::GetIO().DisplaySize.x - windowWidth) / 2.0f;
    float windowPosY = (30.0f);

    ImGui::SetNextWindowPos(ImVec2(windowPosX, windowPosY));
    ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));

    ImGui::Begin("ControlsBar", nullptr, windowFlags);


    // Bot�n PLAY
    if (ImGui::Button("Play", ImVec2(25, 25)))
    {
        if (App->GetIsRunning()) {
            App->SetState(GameState::STOPPED);
            ImGui::SetWindowFocus("Scene");
        }
        else {
            App->SetState(GameState::PLAYING);
            ImGui::SetWindowFocus("Game");
        }
    }

    ImGui::SameLine();

    // Bot�n PAUSE
    if (ImGui::Button("Pause", ImVec2(25, 25)))
    {
        if (App->GetIsRunning()) {
            App->SetState(GameState::PAUSED);
        }
    }

    ImGui::SameLine();

    // Bot�n STOP
    if (ImGui::Button("Stop", ImVec2(25, 25)))
    {
        if (App->GetIsRunning()) {
            App->SetState(GameState::STOPPED);
            ImGui::SetWindowFocus("Scene");
        }
    }

    ImGui::End();
}

void ModuleEditor::ViewCollapsingHeader() {

    if (ImGui::BeginMenu("View")) {

        if (ImGui::Checkbox("Hierarchy", &isActiveHierarchy))
        {
            isActiveHierarchy != isActiveHierarchy;
        }
        if (ImGui::Checkbox("Inspector", &isActiveInspector))
        {
            isActiveInspector != isActiveInspector;
        }
        if (ImGui::Checkbox("Console", &isActiveConsole))
        {
            isActiveConsole != isActiveConsole;
        }
        if (ImGui::Checkbox("Scene Window", &isActiveSceneWindow))
        {
            isActiveSceneWindow != isActiveSceneWindow;
        }
        if (ImGui::Checkbox("Game Window", &isActiveGameWindow))
        {
            isActiveGameWindow != isActiveGameWindow;
        }

        ImGui::EndMenu();
    }
}

void ModuleEditor::HardwareCollapsingHeader()
{
    if (ImGui::CollapsingHeader("Hardware"))
    {
        ImGui::Text(sdlVersion.c_str());
        ImGui::Separator();
        
        ImGui::Text("CPUs ");
        ImGui::SameLine();
        ImGui::Text("%u", cpuCount);
        ImGui::SameLine();
        ImGui::Text(" (Cache: %u", cpuCacheSize);
        ImGui::SameLine();
        ImGui::Text("kb)\n");


        ImGui::Text("System RAM: ");
        ImGui::SameLine();
        ImGui::TextColored(IMGUICOL_SKY_BLUE, "%.2f", systemRAM);
        ImGui::SameLine();
        ImGui::TextColored(IMGUICOL_SKY_BLUE, "Gb");

        ImGui::Separator();
        ImGui::Text("Sceen Devices:");

        for (const std::string& screen : screens) {
            ImGui::Text("%s", screen.c_str());
        }

        ImGui::Text("Caps: ");

        if (Now3D) { ImGui::SameLine(); ImGui::TextColored(IMGUICOL_SKY_BLUE, "3DNOW,"); }
        if (AltiVec) { ImGui::SameLine(); ImGui::TextColored(IMGUICOL_SKY_BLUE, "AltiVec,"); }
        if (AVX) { ImGui::SameLine(); ImGui::TextColored(IMGUICOL_SKY_BLUE, "AVX,"); }
        if (AVX2) { ImGui::SameLine(); ImGui::TextColored(IMGUICOL_SKY_BLUE, "AVX2,"); }
        if (MMX) { ImGui::SameLine(); ImGui::TextColored(IMGUICOL_SKY_BLUE, "MMX,"); }
        if (RDTSC) { ImGui::SameLine(); ImGui::TextColored(IMGUICOL_SKY_BLUE, "RDTSC,"); }
        if (SSE) { ImGui::SameLine(); ImGui::TextColored(IMGUICOL_SKY_BLUE, "SSE,"); }
        if (SSE2) { ImGui::SameLine(); ImGui::TextColored(IMGUICOL_SKY_BLUE, "SSE2,"); }
        if (SSE3) { ImGui::SameLine(); ImGui::TextColored(IMGUICOL_SKY_BLUE, "SSE3,"); }
        if (SSE41) { ImGui::SameLine(); ImGui::TextColored(IMGUICOL_SKY_BLUE, "SSE41,"); }
        if (SSE42) { ImGui::SameLine(); ImGui::TextColored(IMGUICOL_SKY_BLUE, "SSE42"); }

    }
}

void ModuleEditor::WindowCollapsingHeader()
{
    if (ImGui::CollapsingHeader("Window"))
    {
        if (ImGui::BeginTable("split", 3))
        {
            int oldWidth = App->window->width;
            int oldHeight = App->window->height;

            ImGui::TableNextColumn();
            if (ImGui::Checkbox("Fullscreen", &fullscreen))
            {
                App->window->SetFullscreen(fullscreen);
            }

            ImGui::TableNextColumn();
            if (ImGui::Checkbox("Resizable", &resizable))
            {
                App->window->SetResizable(resizable);
            }

            ImGui::TableNextColumn();
            ImGui::SliderInt("Window Width", &App->window->width, 320, 2560);
            
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("Borderless", &borderless))
            {
                App->window->SetBorderless(borderless);
            }
            
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("Full Desktop", &fulldesktop))
            {
                App->window->SetFulldesktop(fulldesktop);
            }

   
            ImGui::TableNextColumn();
            ImGui::SliderInt("Window Height", &App->window->height, 256, 2048);

            if (oldWidth != App->window->width) { App->window->OnWidthChanged(); }
            if (oldHeight != App->window->height) { App->window->OnHeightChanged(); }

            ImGui::Spacing();

            if (ImGui::SliderFloat("Brightness", &brightness, 0.100f, 1.000f))
            {
                SDL_SetWindowBrightness(App->window->window, brightness);
                
            }
            ImGui::EndTable();
        }
    }
}

void ModuleEditor::RenderCollapsingHeader()
{
    if (ImGui::CollapsingHeader("Renderer"))
    {
        if (ImGui::BeginTable("split", 3))
        {
            ImGui::TableNextColumn();
            if (ImGui::Checkbox("GL_DEPTH_TEST", &depthTest))
            {
                App->renderer3D->SetDepthTestAttribute(depthTest);
            }

            ImGui::TableNextColumn();
            if (ImGui::Checkbox("GL_CULL_FACE", &cullFace))
            {
                App->renderer3D->SetCullFaceAttribute(cullFace);
            }

            ImGui::TableNextColumn();
            if (ImGui::Checkbox("GL_LIGHTING", &lighting))
            {
                App->renderer3D->SetLightingAttribute(lighting);
            }

            ImGui::Spacing();

            ImGui::TableNextColumn();
            if (ImGui::Checkbox("GL_COLOR_MATERIAL", &coloMaterial))
            {
                App->renderer3D->SetColorMaterialAttribute(coloMaterial);
            }

            ImGui::TableNextColumn();
            if (ImGui::Checkbox("GL_TEXTURE_2D", &texture2D))
            {
                App->renderer3D->SetTexture2DAttribute(texture2D);
            }

            ImGui::TableNextColumn();
            if (ImGui::Checkbox("GL_BLEND", &blend))
            {
                App->renderer3D->SetBlendAttribute(blend);
            }

            ImGui::Spacing();

            ImGui::TableNextColumn();
            if (ImGui::Checkbox("GL_ALPHA_TEST", &alphaTest))
            {
                App->renderer3D->SetAlphaTestAttribute(alphaTest);
            }

            ImGui::TableNextColumn();
            if (ImGui::Checkbox("GL_LINE_SMOOTH", &lineSmooth))
            {
                App->renderer3D->SetLineSmoothAttribute(lineSmooth);
            }

            ImGui::TableNextColumn();
            if (ImGui::Checkbox("GL_POINT_SMOOTH", &pointSmooth))
            {
                App->renderer3D->SetPointSmoothAttribute(pointSmooth);
            }

            ImGui::TableNextColumn();
            if (ImGui::Checkbox("GL_POLYGON_SMOOTH", &polygonSmooth))
            {
                App->renderer3D->SetPolygonSmoothAttribute(polygonSmooth);
            }

            ImGui::TableNextColumn();
            if (ImGui::Checkbox("WIREFRAME_MODE", &wireframeMode))
            {
                App->renderer3D->SetWireframeMode(wireframeMode);
            }

            ImGui::Spacing();

   
            ImGui::EndTable();
        }
    }
}

void ModuleEditor::CreateAboutWindow(bool& showAboutWindow)
{
    if (!showAboutWindow)
        return;

    ImGui::SetNextWindowSize(ImVec2(920, 300), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("About", &showAboutWindow))
    {
        showAboutWindow = true;
        ImGui::End();
        return;
    }

    // Basic Info
    ImGui::Text("Nano Engine");
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Venture into the realm of game development with Nano Engine, forked from the \nlegendary Sheesh Engine, a creation born from the collaborative efforts of two \nvisionary minds at CITM.");
    ImGui::Text("Whether you seek to forge epic tales of heroism or weave \nenchanting mysteries, this engine is your magical wand.");
    ImGui::Spacing();
    if (ImGui::Button("Autors: Lluc Estruch Andreu & Rafael Esquius Perez"))
    {
        URLButton("https://github.com/Llucaieaie/NanoEngine");
    }
    if (ImGui::Button("Forked Engine Autors: Oriol Martin Corella & Xiao Shan Costajussa Bellver"))
    {
        URLButton("https://github.com/Urii98/SheeeshEngine");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Libraries
    ImGui::Text("Libraries and links to their websites:");

    if (ImGui::Button("SDL"))
    {
        URLButton("https://libsdl.org/index.php");
    }
    ImGui::SameLine();
    ImGui::Text(sdlVersion.c_str());

    if (ImGui::Button("OpenGL"))
    {
        URLButton("https://www.opengl.org/");
    }
    ImGui::SameLine();
    ImGui::Text("OpenGL: %s", glewGetString(GLEW_VERSION));

    if (ImGui::Button("ImGui"))
    {
        URLButton("https://github.com/ocornut/imgui/");
    }
    ImGui::SameLine();
    ImGui::Text(ImGui::GetVersion());

    if (ImGui::Button("Glew"))
    {
        URLButton("http://glew.sourceforge.net/");
    }
    ImGui::SameLine();
    ImGui::Text((const char*)glewGetString(GLEW_VERSION));

    // Phys
    // Assimp
    if (ImGui::Button("Assimp"))
    {
        URLButton("https://www.assimp.org/");
    }
    ImGui::SameLine();
    ImGui::Text(assimpVersion.c_str());

    
    //Devil
    if (ImGui::Button("DevIL"))
    {
        URLButton("http://openil.sourceforge.net/");
    }
    ImGui::SameLine();
    ImGui::Text("v1.8.0");

    ImGui::Separator();
    ImGui::Spacing();

    // Licence
    ImGui::Text(license.c_str());

    // Close Button
    if (ImGui::Button("Close", ImVec2(60, 0)))
    {
        showAboutWindow = false;
    }

    ImGui::End();



}

void ModuleEditor::CreateConsoleWindow(bool& showConsoleWindow)
{
    if (!showConsoleWindow)
        return;

    bool AutoScroll = false;
    ImGuiTextFilter Filter;

    ImGui::SetNextWindowSize(ImVec2(920, 300), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Console", &showConsoleWindow))
    {
        showConsoleWindow = true;
        ImGui::End();
        return;
    }

    if (ImGui::SmallButton("Clear")) { logs->clear(); }

    ImGui::Separator();
    ImGui::Spacing();

    // Options menu
    if (ImGui::BeginPopup("Options"))
    {
        ImGui::Checkbox("Auto-scroll", &AutoScroll);
        ImGui::EndPopup();
    }

    //Texto    
    for (std::string& str : *(logs))
    {
        ImGui::Text(str.c_str());
    }    

    ImGui::End();
}

void ModuleEditor::GameWindow(bool& isActiveGameWindow)
{
    ImGui::Begin("Game", 0, ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNavFocus);
    sceneWindowSize = ImGui::GetContentRegionAvail();

    ImVec2 newWinSize = sceneWindowSize;
    newWinSize.x = (newWinSize.y / 9.0f) * 16.0f;

    float uvOffset = (sceneWindowSize.x - newWinSize.x) / 2.0f;
    uvOffset /= newWinSize.x;

    if (App->renderer3D->activeCam != nullptr)
        ImGui::Image((ImTextureID)App->renderer3D->activeCam->cameraBuffer, sceneWindowSize, ImVec2(-uvOffset, 1), ImVec2(1 + uvOffset, 0));

    ImGui::End();
}

void ModuleEditor::SceneWindow(bool& isActiveSceneWindow) 
{
    ImGui::Begin("Scene");
    sceneWindowSize = ImGui::GetContentRegionAvail();

    ImVec2 newWinSize = sceneWindowSize;
    newWinSize.x = (newWinSize.y / 9.0f) * 16.0f;

    float uvOffset = (sceneWindowSize.x - newWinSize.x) / 2.0f;
    uvOffset /= newWinSize.x;

    ImGui::Image((ImTextureID)App->camera->camera->cameraBuffer, sceneWindowSize, ImVec2(-uvOffset, 1), ImVec2(1 + uvOffset, 0));
    
    if (ImGui::IsMouseClicked(0) && App->input->GetKey(SDL_SCANCODE_LALT) != KEY_REPEAT && ImGui::IsWindowHovered())
    {
        std::vector<GameObject*> objectsSelectedByMouse;

        // Get mouse position
        ImVec2 mousePosition = ImGui::GetMousePos();

        // Normalize mouse position
        ImVec2 normalizedMousePos = NormMousePos(ImGui::GetWindowPos().x,
            ImGui::GetWindowPos().y + ImGui::GetFrameHeight(),
            ImGui::GetWindowSize().x,
            ImGui::GetWindowSize().y - ImGui::GetFrameHeight(), mousePosition);

        normalizedMousePos.x -= 0.5f;
        normalizedMousePos.y -= 0.5f;

        // Create picking line using the camera frustum
        LineSegment pickingLine = App->camera->camera->frustum.UnProjectLineSegment(normalizedMousePos.x, normalizedMousePos.y);
        App->renderer3D->ray = pickingLine;

        // Iterate through meshes for intersection
        for (size_t i = 0; i < App->assimpMeshes->meshes.size(); i++)
        {
            if (pickingLine.Intersects(App->assimpMeshes->meshes[i]->OBB))
            {
                if (App->assimpMeshes->meshes[i]->owner != nullptr)
                    objectsSelectedByMouse.push_back(App->assimpMeshes->meshes[i]->owner);
            }
        }

        float currentDistance;
        float minDistance = 0;

        // Find closest object
        for (int i = 0; i < objectsSelectedByMouse.size(); i++)
        {
            Mesh* mesh = objectsSelectedByMouse[i]->GetMeshComponent()->mesh;
            float4x4 matrix = objectsSelectedByMouse[i]->transform->getGlobalMatrix().Transposed();

            for (int j = 0; j < mesh->indexCount; j += 3)
            {
                float4 point1, point2, point3;
                float* vertex1 = &mesh->vertex[mesh->index[j] * VERTEX];
                float* vertex2 = &mesh->vertex[mesh->index[j + 1] * VERTEX];
                float* vertex3 = &mesh->vertex[mesh->index[j + 2] * VERTEX];

                point1 = matrix * float4(*vertex1, *(vertex1 + 1), *(vertex1 + 2), 1);
                point2 = matrix * float4(*vertex2, *(vertex2 + 1), *(vertex2 + 2), 1);
                point3 = matrix * float4(*vertex3, *(vertex3 + 1), *(vertex3 + 2), 1);

                float3 _point1, _point2, _point3;
                _point1 = float3(point1.x, point1.y, point1.z);
                _point2 = float3(point2.x, point2.y, point2.z);
                _point3 = float3(point3.x, point3.y, point3.z);

                Triangle triangle(_point1, _point2, _point3);

                if (pickingLine.Intersects(triangle, &currentDistance, nullptr))
                {
                    if (minDistance == 0) {
                        minDistance = currentDistance;
                        App->scene->SetGameObjectSelected(objectsSelectedByMouse[i]);
                        continue;
                    }

                    if (minDistance > currentDistance) {
                        minDistance = currentDistance;
                        App->scene->SetGameObjectSelected(objectsSelectedByMouse[i]);
                    }
                }
            }
        }

        // If no objects selected, clear selection
        if (objectsSelectedByMouse.size() == 0)
            App->scene->SetGameObjectSelected(nullptr);

        objectsSelectedByMouse.clear();
    }
    
    ImGui::End();
}

void ModuleEditor::URLButton(const char* url)
{
    
#ifdef _WIN32
    ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    std::string command = "open " + url;
    system(command.c_str());
#elif defined(__linux__)
    std::string command = "xdg-open " + url;
    system(command.c_str());
#else

    ImGui::Text("Sistema operativo no compatible");
#endif
}

void ModuleEditor::UpdatePlots()
{
    AddMs(1000.0 * App->GetDt());
    AddFPS(App->GetFrameRate());
}


void ModuleEditor::AddFPS(const float aFPS)
{
    if (mFPSLog.size() == mFPSLog.capacity())
    {
        for (unsigned int i = 0; i < mFPSLog.size(); i++)
        {
            if (i + 1 < mFPSLog.size())
            {
                float iCopy = mFPSLog[i + 1];
                mFPSLog[i] = iCopy;
            }
        }
        mFPSLog[mFPSLog.capacity() - 1] = aFPS;
    }
    else
    {
        mFPSLog.push_back(aFPS);
    }
}

void ModuleEditor::AddMs(const float aFPS)
{

    if (mMsLog.size() == mMsLog.capacity())
    {

        for (unsigned int i = 0; i < mMsLog.size(); i++)
        {
            if (i + 1 < mMsLog.size())
            {
                float iCopy = mMsLog[i + 1];
                mMsLog[i] = iCopy;
            }
        }
        mMsLog[mMsLog.capacity() - 1] = aFPS;

    }
    else
    {
        mMsLog.push_back(aFPS);
    }
}


std::string ModuleEditor::ReadFileIO(const char* filePath)
{
    std::ifstream file(filePath); 
    std::string contenido; 

    if (file.is_open()) {
        std::string linea;
        while (std::getline(file, linea)) {
            contenido += linea; 
            contenido += '\n'; 
        }
        file.close(); 
    }
    else {
        return std::string();
    }

    return contenido;
}

float ModuleEditor::AverageValueFloatVector(const std::vector<float>& fps)
{
    if (fps.empty()) {
        return std::numeric_limits<float>::quiet_NaN();
    }

    float total = 0.0f;
    for (float value : fps) {
        total += value;
    }

    float average = total / fps.size();
    return std::round(average * 10) / 10.0f;
}



void ModuleEditor::LOGToConsole(const char* text) {

    if (logs == nullptr) return;

    
    std::time_t tiempoActual = std::time(nullptr);

    
    struct std::tm* tiempoDescompuesto = std::localtime(&tiempoActual);

    
    int horas = tiempoDescompuesto->tm_hour;
    int minutos = tiempoDescompuesto->tm_min;
    int segundos = tiempoDescompuesto->tm_sec;

    char horaFormateada[100];
    std::strftime(horaFormateada, sizeof(horaFormateada), "%H:%M:%S ", tiempoDescompuesto);

    std::string aux = horaFormateada;
    aux += text;

    logs->push_front(aux);

}

ImVec2 ModuleEditor::NormMousePos(float x, float y, float w, float h, ImVec2 p)
{
    ImVec2 normP;

    normP.x = (p.x - x) / w;
    normP.y = 1.0f - (p.y - y) / h;
    return normP;
}