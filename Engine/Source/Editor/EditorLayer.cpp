#include "EditorLayer.h"
#include "Runtime/Scene/SceneSerializer.h"
#include "Runtime/Utils/PlatformUtils.h"
#include "Runtime/Utils/MathUtils/MathUtils.h"
#include "Runtime/Utils/Procedural/TerrainNoise.h"
#include "Runtime/EcsFramework/Component/Prop/PropComponent.h"
#include "Runtime/EcsFramework/System/Game/GameSystem.h"
#include "Runtime/Renderer/TextRenderer.h"
#include "Runtime/Audio/AudioSystem.h"
#include "Runtime/Resource/ConfigManager/ConfigManager.h"
#include "Runtime/Resource/AssetManager/AssetManager.h"

#include <glad/glad.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <ImGuizmo.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>


namespace XLEngine
{
    // Window
    static bool bShowViewport = true;
    static bool bShowContentBrowser = true;
    static bool bShowSceneHierachy = true;
    static bool bShowProperties = true;
    static bool bShowStats = true;
    static bool bShowSettings = true;

    // Help
    static bool bShowTutorial = false;
    static bool bShowAboutMe = false;
    static bool bShowDemoImGui = false;

	EditorLayer::EditorLayer()
		:Layer("EditorLayer")
	{

	}

    void EditorLayer::OnAttach()
    {
        XL_CORE_INFO("OnAttach: begin");
        m_CheckerboardTexture = Texture2D::Create(AssetManager::GetInstance().GetFullPath("Assets/textures/Checkerboard.png"));
        m_IconPlay = Texture2D::Create(AssetManager::GetInstance().GetFullPath("Resources/Icons/PlayButton.png"));
        m_IconStop = Texture2D::Create(AssetManager::GetInstance().GetFullPath("Resources/Icons/StopButton.png"));

        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { 
            FramebufferTextureFormat::RGBA8,       // 0: color
            FramebufferTextureFormat::RED_INTEGER, // 1: entity id
            FramebufferTextureFormat::RGBA16F,     // 2: world normal (ink outline)
            FramebufferTextureFormat::DEPTH32F 
        };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);

        // Post-processing framebuffer (styled output shown in the viewport)
        FramebufferSpecification postSpec;
        postSpec.Attachments = {
            FramebufferTextureFormat::RGBA8,
            FramebufferTextureFormat::DEPTH32F
        };
        postSpec.Width = 1280;
        postSpec.Height = 720;
        m_PostFramebuffer = Framebuffer::Create(postSpec);

        m_PostProcessShader = Shader::Create(AssetManager::GetInstance().GetFullPath("Shaders/PostProcess.glsl"));
        XL_CORE_INFO("OnAttach: post shader created");

        // Fullscreen triangle for the post-processing pass
        float screenVertices[] = {
            // pos            // uv
            -1.0f, -1.0f,      0.0f, 0.0f,
             3.0f, -1.0f,      2.0f, 0.0f,
            -1.0f,  3.0f,      0.0f, 2.0f,
        };
        m_ScreenQuadVA = VertexArray::Create();
        Ref<VertexBuffer> screenVB = VertexBuffer::Create(screenVertices, sizeof(screenVertices), VertexBufferUsage::Static);
        screenVB->SetLayout({
            { ShaderDataType::Float2, "a_Pos"     },
            { ShaderDataType::Float2, "a_TexCoord" },
        });
        m_ScreenQuadVA->AddVertexBuffer(screenVB);
        uint32_t screenIndices[] = { 0, 1, 2 };
        Ref<IndexBuffer> screenIB = IndexBuffer::Create(3);
        screenIB->SetData(screenIndices, 3);
        m_ScreenQuadVA->SetIndexBuffer(screenIB);

        m_ActiveScene = CreateRef<Level>();
        m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
        // M1 pure-visual demo scene: 2.5D oblique framing of the procedural terrain
        m_EditorCamera.SetDistance(135.0f);
        m_EditorCamera.SetPitch(52.0f);
        m_EditorCamera.SetYaw(45.0f);

        // Set the window title-bar / taskbar icon from an Asset-folder PNG
        Application::GetInstance().GetWindow().SetTitleIcon(
            AssetManager::GetInstance().GetFullPath("Assets/Textures/SiluokayiLogo.png").string());

        // Procedural terrain (CPU-generated mesh + 6-color palette, zero assets)
        Entity terrain = m_ActiveScene->CreateEntity("Procedural Terrain");
        terrain.AddComponent<TerrainComponent>(); // OnComponentAdded generates the mesh
        XL_CORE_INFO("OnAttach: terrain generated");

        // Procedural props: 45 trees / 6 ruins / 25 rocks, M0-consistent scatter (zero assets)
        {
            XL_CORE_INFO("OnAttach: scatter begin");
            TerrainNoise sNoise(20260829u);
            auto hAt = [&](float x, float z) { return TerrainHeightAt(sNoise, x, z); };

            // M0 LCG scatter RNG (seed 7): deterministic, reproducible scene layout
            uint32_t seed = 7u;
            auto srand = [&]() -> float {
                seed = (seed * 16807u) % 2147483647u;
                return (float)seed / 2147483647.0f;
            };

            // Special-variant trees anchored near the walkway (bent / fallen / root) for near-view variety
            const struct { float x, z; int v; } treeAnchors[] = {
                { -12.0f, 12.0f, 1 }, { 7.0f, -14.0f, 3 }, { 15.0f, 5.0f, 2 },
            };
            int treeCount = 0;
            for (const auto& a : treeAnchors)
            {
                Entity e = m_ActiveScene->CreateEntity("Tree");
                e.AddComponent<PropComponent>(a.x, a.z, (PropType)a.v, 1.15f);
                treeCount++;
            }

            // Remaining trees: random scatter in ±64.5, on higher ground, away from center
            int tries = 0;
            while (treeCount < 45 && tries < 500)
            {
                tries++;
                const float x = (srand() - 0.5f) * 150.0f * 0.86f;
                const float z = (srand() - 0.5f) * 150.0f * 0.86f;
                const float h = hAt(x, z);
                const float d = std::hypot(x, z);
                if (h > 1.2f && d > 14.0f)
                {
                    Entity e = m_ActiveScene->CreateEntity("Tree");
                    e.AddComponent<PropComponent>(x, z, (PropType)(int)(srand() * 4.0f), 0.8f + srand() * 1.5f);
                    treeCount++;
                }
            }
            XL_CORE_INFO("OnAttach: trees done, count={0}", treeCount);

            // Rocks: 25 in ±67.5, away from center, scaled 1.2..3.4
            int rockCount = 0;
            tries = 0;
            while (rockCount < 25 && tries < 400)
            {
                tries++;
                const float x = (srand() - 0.5f) * 150.0f * 0.9f;
                const float z = (srand() - 0.5f) * 150.0f * 0.9f;
                const float d = std::hypot(x, z);
                if (d > 18.0f)
                {
                    Entity e = m_ActiveScene->CreateEntity("Rock");
                    e.AddComponent<PropComponent>(x, z, PropType::Rock, 1.2f + srand() * 2.2f);
                    rockCount++;
                }
            }
            XL_CORE_INFO("OnAttach: rocks done, count={0}", rockCount);

            // Ruins: 6 — 4 fixed anchors (in-frame) + 2 random, avoiding the toxic glow point (24,-18)
            const struct { float x, z; int t; } ruinAnchors[] = {
                { -9.0f, 7.0f, 0 }, { 13.0f, 12.0f, 1 }, { -21.0f, -8.0f, 0 }, { 2.0f, -20.0f, 1 },
            };
            int ruinCount = 0;
            for (const auto& a : ruinAnchors)
            {
                Entity e = m_ActiveScene->CreateEntity("Ruins");
                e.AddComponent<PropComponent>(a.x, a.z, (PropType)(4 + a.t), 1.0f); // 4=RuinsMetal, 5=RuinsRubble
                ruinCount++;
            }
            tries = 0;
            while (ruinCount < 6 && tries < 200)
            {
                tries++;
                const float x = (srand() - 0.5f) * 150.0f * 0.6f;
                const float z = (srand() - 0.5f) * 150.0f * 0.6f;
                const float dLight = std::hypot(x - 24.0f, z + 18.0f);
                const float dC = std::hypot(x, z);
                if (dLight > 18.0f && dC > 12.0f)
                {
                    Entity e = m_ActiveScene->CreateEntity("Ruins");
                    e.AddComponent<PropComponent>(x, z, (PropType)(4 + (int)(srand() * 2.0f)), 1.0f);
                    ruinCount++;
                }
            }
            XL_CORE_INFO("OnAttach: ruins done, count={0}", ruinCount);
        }

        m_SceneHierarchyPanel.SetContext(m_ActiveScene);

#if 0
        // Entity
        Entity square = m_ActiveScene->CreateEntity("Green Square");
        square.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.0f,1.0f,0.0f,1.0f });
       
        Entity redSquare = m_ActiveScene->CreateEntity("Red Square");
        redSquare.AddComponent<SpriteRendererComponent>(glm::vec4{ 1.0f,0.0f,0.0f,1.0f });

        m_SquareEntity = square;

        m_CameraEntity = m_ActiveScene->CreateEntity("Camera A");
        m_CameraEntity.AddComponent<CameraComponent>();

        m_SecondCamera = m_ActiveScene->CreateEntity("Camera B");
        m_SecondCamera.AddComponent<CameraComponent>().Primary = false;

        class CameraController :public ScriptableEntity
        {
        public:
            void OnCreate()
            {
                auto& translation = GetComponent<TransformComponent>().Translation;
                translation.x = rand() % 10 - 5.0f;
            }

            void OnDestory()
            {

            }

            void OnUpdate(Timestep ts)
            {
                auto& translation = GetComponent<TransformComponent>().Translation;
                float speed = 5.0f;

                if (Input::IsKeyPressed(KeyCode::A))
                    translation.x -= speed * ts;
                if (Input::IsKeyPressed(KeyCode::D))
                    translation.x += speed * ts;
                if (Input::IsKeyPressed(KeyCode::W))
                    translation.y += speed * ts;
                if (Input::IsKeyPressed(KeyCode::S))
                    translation.y -= speed * ts;
            }
        };

        m_CameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();
        m_SecondCamera.AddComponent<NativeScriptComponent>().Bind<CameraController>();
#endif

        // P1-1 内置 5x7 点阵字体纹理（HUD / 提示用）
        TextRenderer::Init();

        // P1-4 WinMM 合成音频（环境氛音在 Init 内起播）
        AudioSystem::Init();
    }

    

    void EditorLayer::OnDetach()
    {
        // P1-4 停止环境音并释放缓冲
        AudioSystem::Shutdown();
    }

    void EditorLayer::OnUpdate(Timestep ts)
    {
        XL_PROFILE_FUNCTION();

        // Resize
        if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
            m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
            (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
        {
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_PostFramebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

            m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }

        m_Framebuffer->ClearAttachment(1, -1);

        m_EditorCamera.SetFlyMode(m_FlyMode);
        m_EditorCamera.SetViewportActive(m_ViewportHovered || m_ViewportFocused);
        m_EditorCamera.OnUpdate(ts);

        // Day-night cycle: advance clock and drive the toon lighting + sky
        if (m_AutoDayNight)
        {
            m_DayTime += ts * m_DayNightSpeed;
            if (m_DayTime >= 1.0f)
                m_DayTime -= 1.0f;
        }
        Renderer3D::SetTime(m_DayTime);

        // Render
        Renderer2D::ResetStats();

        {
            XL_PROFILE_SCOPE("Renderer Prep");
            m_Framebuffer->Bind();
            RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
            RenderCommand::Clear();
        }

        // Clear out entity ID attachment to -1
        m_Framebuffer->ClearAttachment(1, -1);

        /*{
            static float rotation = 0.0f;
            rotation += ts * 50.0f;

            XL_PROFILE_SCOPE("Renderer Draw");
            Renderer2D::BeginScene(m_CameraController.GetCamera());
            Renderer2D::DrawRotatedQuad({ 1.0f, 0.0f }, { 0.8f, 0.8f }, glm::radians(-45.0f), { 0.8f, 0.2f, 0.3f, 1.0f });
            Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, { 0.8f, 0.2f, 0.3f, 1.0f });
            Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.2f, 0.3f, 0.8f, 1.0f });
            Renderer2D::DrawQuad({ 0.0f,  0.0f, -0.1f }, { 20.0f, 20.0f }, m_CheckerboardTexture, 10.0f);
            Renderer2D::DrawRotatedQuad({ -2.0f,  0.0f,  0.0f }, { 1.0f, 1.0f }, glm::radians(rotation), m_CheckerboardTexture, 20.0f);
            Renderer2D::EndScene();

            Renderer2D::BeginScene(m_CameraController.GetCamera());
            for (float y = -5.0f; y < 5.0f; y += 0.5f)
            {
                for (float x = -5.0f; x < 5.0f; x += 0.5f)
                {
                    glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
                    Renderer2D::DrawQuad({ x, y }, { 0.45f, 0.45f }, color);
                }
            }
            Renderer2D::EndScene();
            m_Framebuffer->Unbind();
        }*/

        if (ModeManager::IsEditState())
        {
            m_EditorCamera.OnUpdate(ts);
            m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
        }
        else
        {
            m_ActiveScene->OnUpdateRuntime(ts);
        }

        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;
        glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
        my = viewportSize.y - my;
        int mouseX = (int)mx;
        int mouseY = (int)my;

        if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
        {
            int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
            m_HoveredEntity = pixelData == -1 ? Entity{} : Entity{ (entt::entity)pixelData, m_ActiveScene.get() };
        }
        
        OnOverlayRender();

        // ---- P1-1 HUD: screen-space bitmap text over the 3D scene (drawn into the color framebuffer) ----
        {
            const auto& fboSpec = m_Framebuffer->GetSpecification();
            const float H = (float)fboSpec.Height;

            const glm::vec4 pale  = { 0.788f, 0.784f, 0.722f, 1.0f }; // #C9C8B8 惨白
            const glm::vec4 ember = { 0.788f, 0.431f, 0.227f, 1.0f }; // #C96E3A 余烬橙
            const glm::vec4 mote  = { 0.616f, 0.722f, 0.290f, 1.0f }; // #9DB84A 荧绿

            if (GameSystem* gs = m_ActiveScene ? m_ActiveScene->GetGameSystem() : nullptr)
            {
                // Close depth test so HUD stays on top of terrain / props.
                glDisable(GL_DEPTH_TEST);
                TextRenderer::BeginScene(fboSpec.Width, fboSpec.Height);

                // 光尘进度（顶栏，惨白）
                std::string motes = "MOTES  " + std::to_string(gs->GetMotesCollected())
                                  + "/" + std::to_string(gs->GetMotesTotal());
                TextRenderer::DrawString(motes, 14.0f, H - 7.0f * 2.0f - 14.0f, 2.0f, pale);

                // 黎明达成（顶栏下方，荧绿）
                if (gs->IsDawn())
                    TextRenderer::DrawString("DAWN  HAS  COME", 14.0f, H - 7.0f * 2.0f * 2.0f - 26.0f, 1.5f, mote);

                // 底部操作提示（余烬橙）
                TextRenderer::DrawString("WASD  MOVE    E  LIGHT  BEACON", 14.0f, 14.0f, 1.5f, ember);

                TextRenderer::EndScene();
                glEnable(GL_DEPTH_TEST);
            }
        }

        m_Framebuffer->Unbind();

        // ---- Post-processing pass: ink outline / grain / grade / vignette / depth fog ----
        {
            m_PostFramebuffer->Bind();
            RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
            RenderCommand::Clear();

            m_PostProcessShader->Bind();
            m_PostProcessShader->SetInt("u_SceneColor", 0);
            m_PostProcessShader->SetInt("u_SceneDepth", 1);
            m_PostProcessShader->SetInt("u_SceneNormal", 2);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_Framebuffer->GetColorAttachmentRendererID(0));
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_Framebuffer->GetDepthAttachmentRendererID());
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, m_Framebuffer->GetColorAttachmentRendererID(2));

            // Stylized post-processing parameters (art-direction locked)
            m_PostProcessShader->SetFloat("u_OutlineStrength", 1.0f);
            m_PostProcessShader->SetFloat("u_GrainAmount", 0.028f);
            m_PostProcessShader->SetFloat("u_Contrast", 1.08f);
            m_PostProcessShader->SetFloat("u_Saturation", 0.82f);
            m_PostProcessShader->SetFloat("u_Brightness", 1.0f);
            m_PostProcessShader->SetFloat("u_Vignette", 0.85f);
            m_PostProcessShader->SetFloat("u_FogStrength", 0.4f);
            m_PostProcessShader->SetFloat("u_Near", 0.1f);
            m_PostProcessShader->SetFloat("u_Far", 1000.0f);

            // Viewport mode + day-night / sky-parallax uniforms for the post pass
            m_PostProcessShader->SetInt("u_ShowDiagnostics", m_ShowDiagnostics ? 1 : 0);
            m_PostProcessShader->SetFloat("u_Time", m_DayTime);
            m_PostProcessShader->SetFloat3("u_CameraPos", m_EditorCamera.GetPosition());
            m_PostProcessShader->SetMat4("u_InvViewProj", glm::inverse(m_EditorCamera.GetViewProjection()));

            m_ScreenQuadVA->Bind();
            RenderCommand::DrawIndexed(m_ScreenQuadVA, 3);
            m_ScreenQuadVA->Unbind();
            m_PostProcessShader->Unbind();
            m_PostFramebuffer->Unbind();
        }
    }

    void EditorLayer::OnImGuiRender()
    {
        XL_PROFILE_FUNCTION();

        // Note: Switch this to true to enable dockspace
        // ----DockSpace Begin----
        static bool dockspaceOpen = true;
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 110.0;

        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        style.WindowMinSize.x = minWinSizeX;

        // ----MenuBar Begin----
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N"))
                    NewScene();

                if (ImGui::MenuItem("Open...", "Ctrl+O"))
                    OpenScene();

                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                    SaveSceneAs();

                if (ImGui::MenuItem("Exit", NULL, false))
                    Application::GetInstance().Close();

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Window"))
            {
                ImGui::MenuItem("Viewport", NULL, &bShowViewport);
                ImGui::MenuItem("Content Browser", NULL, &bShowContentBrowser);
                ImGui::MenuItem("Scene Hierachy", NULL, &bShowSceneHierachy);
                ImGui::MenuItem("Properties", NULL, &bShowProperties);
                ImGui::MenuItem("Stats", NULL, &bShowStats);
                ImGui::MenuItem("Settings", NULL, &bShowSettings);

                if (ImGui::MenuItem("Load Default Layout"))
                    LoadDefaultEditorConfig();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help"))
            {
                ImGui::MenuItem("Tutorial", NULL, &bShowTutorial);
                ImGui::MenuItem("About Me", NULL, &bShowAboutMe);
                ImGui::MenuItem("Demo ImGui", NULL, &bShowDemoImGui);
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
        // ----MenuBar End----

        // ----Windows Begin----
        if (bShowContentBrowser)
        {
            m_ContentBrowserPanel.OnImGuiRender(&bShowContentBrowser);
        }
        if (bShowSceneHierachy || bShowProperties)
        {
            m_SceneHierarchyPanel.OnImGuiRender(&bShowSceneHierachy, &bShowProperties);
        }

        // ----Stats Begin----
        if (bShowStats)
        {
            ImGui::Begin("Stats");

            std::string name = "None";
            if (m_HoveredEntity)
                name = m_HoveredEntity.GetComponent<TagComponent>().Tag;

            ImGui::Text("Hovered Entity: %s", name.c_str());

            auto stats = Renderer2D::GetStats();
            ImGui::Text("Renderer2D Stats:");
            ImGui::Text("Draw Calls: %d", stats.DrawCalls);
            ImGui::Text("Quads: %d", stats.QuadCount);
            ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
            ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
            ImGui::End();
        }
        // ----Stats End----

        if (bShowSettings)
        {
            ImGui::Begin("Settings", &bShowSettings);
            ImGui::Checkbox("Show physics colliders", &m_ShowPhysicsColliders);
            ImGui::Separator();
            ImGui::Text("M1 Stylized Viewport");
            ImGui::Checkbox("Diagnostic split view", &m_ShowDiagnostics);
            ImGui::Checkbox("Auto day-night cycle", &m_AutoDayNight);
            if (!m_AutoDayNight)
                ImGui::SliderFloat("Time of day", &m_DayTime, 0.0f, 1.0f);
            ImGui::Separator();
            ImGui::Text("Camera");
            ImGui::Checkbox("Roam mode (FPS)", &m_FlyMode);
            ImGui::End();
        }

        if (bShowViewport)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
            ImGui::Begin("Viewport");

            auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
            auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
            auto viewportOffset = ImGui::GetWindowPos();
            m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
            m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

            m_ViewportFocused = ImGui::IsWindowFocused();
            m_ViewportHovered = ImGui::IsWindowHovered();
            Application::GetInstance().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);


            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

            uint32_t textureID = m_PostFramebuffer->GetColorAttachmentRendererID(); // styled output (outline/grain/grade/fog)
            ImGui::Image((void*)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const wchar_t* path = (const wchar_t*)payload->Data;
                    OpenScene(std::filesystem::path(ConfigManager::GetInstance().GetAssetsFolder()) / path);
                }
                ImGui::EndDragDropTarget();
            }

            // Roam-mode control hint (overlay, top-left of the viewport)
            if (m_FlyMode)
            {
                ImGui::SetCursorScreenPos(ImVec2(m_ViewportBounds[0].x + 10.0f, m_ViewportBounds[0].y + 10.0f));
                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f),
                    "ROAM  [WASD] move  [Space] up  [Q] down  [RMB] look  [Shift] sprint  [F] exit");
            }

            // Gizmos
            Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
            if (selectedEntity && m_GizmoType != -1)
            {
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();
                ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

                // Camera
                // Runtime camera from entity
               /* auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
                const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
                const glm::mat4& cameraProjection = camera.GetProjection();
                glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());*/

                // Editor camera
                const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
                glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

                // Entity transform
                auto& tc = selectedEntity.GetComponent<TransformComponent>();
                glm::mat4 transform = tc.GetTransform();

                // Snapping
                bool snap = Input::IsKeyPressed(Key::LeftControl);
                float snapValue = 0.5f; // Snap to 0.5m for translation/scale
                // Snap to 45 degrees for rotation
                if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
                    snapValue = 45.0f;

                float snapValues[3] = { snapValue, snapValue, snapValue };

                ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                    (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
                    nullptr, snap ? snapValues : nullptr);

                if (ImGuizmo::IsUsing())
                {
                    glm::vec3 translation, rotation, scale;
                    Math::DecomposeTransform(transform, translation, rotation, scale);

                    glm::vec3 deltaRotation = rotation - tc.Rotation;
                    tc.Translation = translation;
                    tc.Rotation += deltaRotation;
                    tc.Scale = scale;
                }
            }

            ImGui::End();
            ImGui::PopStyleVar();
        }
        // ----Windows End----

        // ----Help Begin----
        // TODO
        ImGuiWindowFlags helpMenuFlags = ImGuiWindowFlags_NoDocking;
        if (bShowTutorial)
        {
            ImGui::Begin("Tutorial", &bShowTutorial, helpMenuFlags);
            ImGui::Text("Hello!");
            ImGui::Text("Hello!");
            ImGui::End();
        }
        if (bShowAboutMe)
        {
            ImGui::Begin("About Me", &bShowAboutMe, helpMenuFlags);
            ImGui::Text("My name is Lynn");
            ImGui::End();
        }
        if (bShowDemoImGui)
        {
            ImGui::ShowDemoWindow(&bShowDemoImGui);
        }
        // ----Help End----
        UI_Toolbar();

        ImGui::End();
        // ----DockSpace End----
    }

    void EditorLayer::UI_Toolbar()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        auto& colors = ImGui::GetStyle().Colors;
        const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
        const auto& buttonActive = colors[ImGuiCol_ButtonActive];
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

        ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        float size = ImGui::GetWindowHeight() - 4.0f;
        Ref<Texture2D> icon = ModeManager::IsEditState() ? m_IconPlay : m_IconStop;
        ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
        if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
        {
            if (ModeManager::IsEditState())
                OnScenePlay();
            else
                OnSceneStop();
        }
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
        ImGui::End();
    }

    void EditorLayer::LoadDefaultEditorConfig()
    {
        const std::filesystem::path CurrentEditorConfigPath{ AssetManager::GetInstance().GetFullPath("imgui.ini") };
        const std::filesystem::path DefaultEditorConfigPath{ AssetManager::GetInstance().GetFullPath("Assets/Config/imgui.ini")};
        XL_CORE_ASSERT(std::filesystem::exists(DefaultEditorConfigPath));
        if (std::filesystem::exists(CurrentEditorConfigPath))
            std::filesystem::remove(CurrentEditorConfigPath);
        std::filesystem::copy(DefaultEditorConfigPath, std::filesystem::current_path());

        bShowViewport = true;
        bShowContentBrowser = true;
        bShowSceneHierachy = true;
        bShowProperties = true;
        bShowStats = true;
        bShowSettings = true;

        // seems imgui docking branch has some bugs with load ini file?

        //auto& io = ImGui::GetIO();
        //io.IniFilename = DefaultEditorConfigPath.string().c_str();
        //ImGui::LoadIniSettingsFromDisk(DefaultEditorConfigPath.string().c_str());
        //ImGui::DockContextRebuildNodes(ImGui::GetCurrentContext());
    }

    void EditorLayer::OnEvent(Event& e)
    {
        m_EditorCamera.OnEvent(e);
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(XL_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(XL_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
    }

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        // Shortcuts
        if (e.GetRepeatCount() > 0)
            return false;

        bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
        bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
        switch (e.GetKeyCode())
        {
        case Key::N:
        {
            if (control)
                NewScene();
            break;
        }
        case Key::O:
        {
            if (control)
                OpenScene();
            break;
        }
        case Key::S:
        {
            if (control)
            {
                if (shift)
                    SaveSceneAs();
                else
                    SaveScene();
            }
            break;
        }
        // Scene Commands
        case Key::D:
        {
            if (control)
                OnDuplicateEntity();
            break;
        }

        // Gizmos (disabled while roaming so W/A/S/D/Q drive the camera instead of the gizmo)
        case Key::Q:
            if (!m_FlyMode)
                m_GizmoType = -1;
            break;
        case Key::W:
            if (!m_FlyMode)
                m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
            break;
        case Key::E:
            if (!m_FlyMode)
                m_GizmoType = ImGuizmo::OPERATION::ROTATE;
            break;
        case Key::R:
            if (!m_FlyMode)
                m_GizmoType = ImGuizmo::OPERATION::SCALE;
            break;

        // Camera roam
        case Key::F:
            if (!ImGui::GetIO().WantTextInput)
                m_FlyMode = !m_FlyMode;
            break;
        }

       
    }

    bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        if (e.GetMouseButton() == Mouse::ButtonLeft)
        {
            if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt) && m_HoveredEntity)
                m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
        }
        return false;
    }

    void EditorLayer::OnOverlayRender()
    {
        if (ModeManager::IsEditState())
        {
            Renderer2D::BeginScene(m_EditorCamera);
        }
        else
        {
            Entity camera = m_ActiveScene->GetPrimaryCameraEntity();
            Renderer2D::BeginScene(camera.GetComponent<CameraComponent>().Camera, camera.GetComponent<TransformComponent>().GetTransform());
        }

        if (m_ShowPhysicsColliders)
        {
            {
                auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
                for (auto entity : view)
                {
                    auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);

                    glm::vec3 translation = tc.Translation + glm::vec3(bc2d.Offset, 0.001f);
                    glm::vec3 scale = tc.Scale * glm::vec3(bc2d.Size * 2.0f, 1.0f);

                    glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
                        * glm::rotate(glm::mat4(1.0f), tc.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
                        * glm::scale(glm::mat4(1.0f), scale);

                    //glm::mat4 transform = glm::translate(tc.GetTransform(), glm::vec3(0, 0, 0.01f));

                    Renderer2D::DrawRect(transform, glm::vec4(0, 1, 0, 1));
                }
            }

            {
                auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
                for (auto entity : view)
                {
                    auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(entity);

                    glm::vec3 translation = tc.Translation + glm::vec3(cc2d.Offset, 0.001f);
                    glm::vec3 scale = tc.Scale * glm::vec3(cc2d.Radius * 2.0f);

                    glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
                        * glm::scale(glm::mat4(1.0f), scale);

                    //glm::mat4 transform = glm::translate(tc.GetTransform(), glm::vec3(0, 0, 0.01f));

                    Renderer2D::DrawCircle(transform, glm::vec4(0, 1, 0, 1), 0.05f);
                }
            }

            Renderer2D::EndScene();
        }
    }

    void EditorLayer::NewScene()
    {
        m_ActiveScene = CreateRef<Level>();
        m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);

        m_EditorScenePath = std::filesystem::path();
    }

    void EditorLayer::OpenScene()
    {
        std::string filepath = FileDialogs::OpenFile("XLEngine Scene (*.xl)\0*.xl\0");
        if (!filepath.empty())
            OpenScene(filepath);
    }
    void EditorLayer::OpenScene(const std::filesystem::path& path)
    {
        if (!ModeManager::IsEditState())
            OnSceneStop();

        if (path.extension().string() != ".xl")
        {
            XL_WARN("Could not load {0} - not a scene file", path.filename().string());
            return;
        }

        Ref<Level> newScene = CreateRef<Level>();
        SceneSerializer serializer(newScene);
        if (serializer.Deserialize(path.string()))
        {
            m_EditorScene = newScene;
            m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_SceneHierarchyPanel.SetContext(m_EditorScene);

            m_ActiveScene = m_EditorScene;
            m_EditorScenePath = path;
        }
    }

    void EditorLayer::SaveScene()
    {
        if (!m_EditorScenePath.empty())
            SerializeScene(m_ActiveScene, m_EditorScenePath);
        else
            SaveSceneAs();
    }

    void EditorLayer::SaveSceneAs()
    {
        std::string filepath = FileDialogs::SaveFile("XLEngine Scene (*.xl)\0*.xl\0");
        if (!filepath.empty())
        {
            SerializeScene(m_ActiveScene, filepath);
            m_EditorScenePath = filepath;
        }
    }

    void EditorLayer::SerializeScene(Ref<Level> scene, const std::filesystem::path& path)
    {
        SceneSerializer serializer(scene);
        serializer.Serialize(path.string());
    }

    void EditorLayer::OnScenePlay()
    {
        if (ModeManager::IsEditState())
            ModeManager::ChangeState();

        m_ActiveScene = Level::Copy(m_EditorScene);
        m_ActiveScene->OnRuntimeStart();

        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
    }

    void EditorLayer::OnSceneStop()
    {
        if (!ModeManager::IsEditState())
            ModeManager::ChangeState();

        m_ActiveScene->OnRuntimeStop();
        m_ActiveScene = m_EditorScene;

        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
    }

    void EditorLayer::OnDuplicateEntity()
    {
        if (!ModeManager::IsEditState())
            return;

        Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity)
            m_EditorScene->DuplicateEntity(selectedEntity);

    }
}