#include "z0/vulkan/vulkan_device.hpp"
#include "z0/log.hpp"
#include "z0/viewport.hpp"
#include "z0/application.hpp"

namespace z0 {
    static void check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        std::cout << "[vulkan] Error: VkResult\n" << std::endl;
        if (err < 0)
            abort();
    }
    void DebugUI::drawUI() {
        statsPanel();
        quitButton();
    }

    void DebugUI::defaultStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
        style.FrameRounding = 4.0f;
        style.WindowPadding = { 2.0f, 2.0f };
        style.WindowRounding = 4.0f;
    }

    void DebugUI::statsPanel() {
        ImGui::Begin("##FPS", nullptr, invisibleWindowFlags);
        ImGui::Text("FPS %.0f", Application::getViewport().getFPS());
        ImGui::SetWindowPos(ImVec2(windowHelper.getWidth() - ImGui::GetWindowWidth() , 0), ImGuiCond_Always);
        ImGui::End();
    }

    void DebugUI::quitButton() {
        ImGui::Begin("##QUIT", nullptr, invisibleWindowFlags);
        ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        if (ImGui::Button("Quit")) {
            windowHelper.close();
        }
        ImGui::End();
    }

    // https://vkguide.dev/docs/extra-chapter/implementing_imgui/
    DebugUI::DebugUI(VulkanDevice& vulkanDevice, WindowHelper& helper):
        windowHelper(helper) {
        // 1: create descriptor pool for IMGUI
        // the size of the pool is very oversize, but it's copied from imgui demo itself.
        VkDescriptorPoolSize pool_sizes[] =
                {
                        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
                };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        if (vkCreateDescriptorPool(vulkanDevice.getDevice(), &pool_info, nullptr, &imguiPool) != VK_SUCCESS) {
            die("Cannot create descriptor pool");
        }

        auto instance =  vulkanDevice.getInstance().getInstance();
        ImGui_ImplVulkan_LoadFunctions([](const char *function_name, void *vulkan_instance) {
            return vkGetInstanceProcAddr(*(reinterpret_cast<VkInstance *>(vulkan_instance)), function_name);
        }, &instance);

        // 2: initialize imgui library
        //this initializes the core structures of imgui
        ImGui::CreateContext();
        //this initializes imgui for GLFW
        ImGui_ImplGlfw_InitForVulkan(windowHelper.getWindowHandle(), true);
        //this initializes imgui for Vulkan
        QueueFamilyIndices queueFamilyIndices = vulkanDevice.findQueueFamilies(vulkanDevice.getPhysicalDevice(), vulkanDevice.getSurface());

        auto colorFormat = vulkanDevice.getSwapChainImageFormat();
        ImGui_ImplVulkan_InitInfo init_info = {
            .Instance = instance,
            .PhysicalDevice = vulkanDevice.getPhysicalDevice(),
            .Device = vulkanDevice.getDevice(),
            .QueueFamily = queueFamilyIndices.graphicsFamily.value(),
            .Queue = vulkanDevice.getGraphicsQueue(),
            .DescriptorPool = imguiPool,
            .MinImageCount = MAX_FRAMES_IN_FLIGHT,
            .ImageCount = MAX_FRAMES_IN_FLIGHT,
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
            .PipelineCache = VK_NULL_HANDLE,
            .UseDynamicRendering = true,
            .PipelineRenderingCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
                .pNext = VK_NULL_HANDLE,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &colorFormat,
                .depthAttachmentFormat = VK_FORMAT_D16_UNORM,
                .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
            },
            .Allocator = nullptr,
            .CheckVkResultFn = check_vk_result,
        };
        ImGui_ImplVulkan_Init(&init_info);

        ImGui_ImplVulkan_CreateFontsTexture();
        defaultStyle();
    }

    void DebugUI::cleanup(VulkanDevice& device) {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(device.getDevice(), imguiPool, nullptr);
    }

    void DebugUI::drawFrame(VkCommandBuffer commandBuffer, VkImageView targetImageView,VkExtent2D extent) {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        drawUI();
        ImGui::Render();

        VkRenderingAttachmentInfo colorAttachment{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .pNext = nullptr,
                .imageView = targetImageView,
                .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        };
        const VkRenderingInfo renderingInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                .pNext = nullptr,
                .renderArea = {{0, 0}, extent},
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &colorAttachment,
                .pDepthAttachment = nullptr,
                .pStencilAttachment = nullptr
        };
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
        vkCmdEndRendering(commandBuffer);
    }

}