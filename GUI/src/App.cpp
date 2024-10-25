/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** App
*/

#include "App.hpp"

#include "Renderer/GlRenderer/GlRenderer.hpp"
#include "Renderer/IRenderer.hpp"
#include "Utils.hpp"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "glm/common.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "imgui.h"

#include <chrono>
#include <cstdio>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <filesystem>
#include <fstream>

App::App(int port, const std::string& host)
    : _networkManager(host, port), _protocolHandler(_networkManager)
{
    m_renderer = std::make_unique<GlRenderer>();
    m_scene = std::make_shared<IRenderer::Scene>();
    {   // Load the meshes and animations
        loadAllPlayer();

        m_teamIndicatorMesh = std::make_shared<StaticMesh>("assets/cone.obj");

        m_ressourceOffset = {
            {FOOD, {0.2, 0.3, m_tileSize[2] * 1/8}},
            {LINEMATE, {0.50, 0.25, m_tileSize[2] * 2/8}},
            {DERAUMERE, {0.55, 0.25, m_tileSize[2] * 3/8}},
            {SIBUR, {0.55, 0.25, m_tileSize[2] * 4/8}},
            {MENDIANE, {0.55, -0.25, m_tileSize[2] * 5/8}},
            {PHIRAS, {0.55, 0.25, m_tileSize[2] * 6/8}},
            {THYSTAME, {0.55, 0.25, m_tileSize[2] * 7/8}}
        };

        m_ressourceMesh = {
            {FOOD, std::make_shared<StaticMesh>("assets/Ressources/Gonstre.obj")},
            {LINEMATE, std::make_shared<StaticMesh>("assets/Ressources/pink.gltf")},
            {DERAUMERE, std::make_shared<StaticMesh>("assets/Ressources/orange.gltf")},
            {SIBUR, std::make_shared<StaticMesh>("assets/Ressources/blue.gltf")},
            {MENDIANE, std::make_shared<StaticMesh>("assets/Ressources/green.gltf")},
            {PHIRAS, std::make_shared<StaticMesh>("assets/Ressources/red.gltf")},
            {THYSTAME, std::make_shared<StaticMesh>("assets/Ressources/purple.gltf")}
        };

        m_tilesMeshes["white"] = std::make_shared<StaticMesh>("assets/whiteRock.gltf");
        m_tilesMeshes["black"] = std::make_shared<StaticMesh>("assets/greyRock.gltf");
        m_broadcastMesh = std::make_shared<StaticMesh>("assets/broadcast.obj");
        m_eggMesh = std::make_shared<StaticMesh>("assets/egg/scene.gltf");
    }

    static const std::vector<std::string> resIconsFilepaths = {
        "assets/Ressources Icons/gonstre.png",
        "assets/Ressources Icons/pink.png",
        "assets/Ressources Icons/orange.png",
        "assets/Ressources Icons/blue.png",
        "assets/Ressources Icons/green.png",
        "assets/Ressources Icons/pruple.png",
        "assets/Ressources Icons/red.png",
    };

    m_resIcons = Utils::Instance<Utils::ImageLoader, const std::vector<std::string>&>::Get(resIconsFilepaths)->getImages();
    _networkManager.connectToServer();
    sleep(1);
    parseConnectionResponse();
    createTiles();
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
    colors[ImGuiCol_Border]                 = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_Button]                 = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
    colors[ImGuiCol_Separator]              = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_DockingPreview]         = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_DockingEmptyBg]         = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding                     = ImVec2(8.00f, 8.00f);
    style.FramePadding                      = ImVec2(5.00f, 2.00f);
    style.CellPadding                       = ImVec2(6.00f, 6.00f);
    style.ItemSpacing                       = ImVec2(6.00f, 6.00f);
    style.ItemInnerSpacing                  = ImVec2(6.00f, 6.00f);
    style.TouchExtraPadding                 = ImVec2(0.00f, 0.00f);
    style.IndentSpacing                     = 25;
    style.ScrollbarSize                     = 15;
    style.GrabMinSize                       = 10;
    style.WindowBorderSize                  = 1;
    style.ChildBorderSize                   = 1;
    style.PopupBorderSize                   = 1;
    style.FrameBorderSize                   = 1;
    style.TabBorderSize                     = 1;
    style.WindowRounding                    = 7;
    style.ChildRounding                     = 4;
    style.FrameRounding                     = 3;
    style.PopupRounding                     = 4;
    style.ScrollbarRounding                 = 9;
    style.GrabRounding                      = 3;
    style.LogSliderDeadzone                 = 4;
    style.TabRounding                       = 4;
}

void App::loadAllPlayer() {
    std::string playerPath = "assets/Players/";
    std::vector<std::string> folders;
    for (const auto& entry : std::filesystem::directory_iterator(playerPath)) {
        glm::vec3 scale;
        if (entry.is_directory()) {
            std::ifstream scaleFile(playerPath + entry.path().filename().string() + "/" + "scale.txt");
            if (scaleFile.is_open()) {
                std::string line;
                for (int i = 0; std::getline(scaleFile, line) && i < 3; i++) {
                    std::istringstream iss(line);
                    if (i == 0)
                        iss >> scale[0];
                    else if (i == 1)
                        iss >> scale[1];
                    else if (i == 2)
                        iss >> scale[2];
                }
                loadPlayer(entry.path().filename().string(), scale);
                scaleFile.close();
            } else {
                throw std::runtime_error("Failed to open scale.txt file");
            }
        }
    }
}

void App::loadPlayer(const std::string& playerName, glm::vec3 scale) {
    m_playerMeshes[playerName].first = std::make_shared<SkeletalMesh>("assets/Players/" + playerName + "/" + playerName + ".dae");
    m_playerMeshes[playerName].second = scale;
    m_playerAnims[playerName + "Idle"] = std::make_shared<Animation>("assets/Players/" + playerName + "/Idle.dae", m_playerMeshes[playerName].first);
    m_playerAnims[playerName + "Move"] = std::make_shared<Animation>("assets/Players/" + playerName + "/Move.dae", m_playerMeshes[playerName].first);
    m_playerAnims[playerName + "Ritual"] = std::make_shared<Animation>("assets/Players/" + playerName + "/Ritual.dae", m_playerMeshes[playerName].first);
    m_playerAnims[playerName + "Birth"] = std::make_shared<Animation>("assets/Players/" + playerName + "/Birth.dae", m_playerMeshes[playerName].first);
}

void App::createTiles() {
    for (float i = -m_mapSize[0] / 2; i < m_mapSize[0] / 2.0; i++) {
        for (float j = -m_mapSize[1] / 2; j < m_mapSize[1] / 2.0; j++) {
            float randomHight = std::rand() % 10 + m_mapSize[0] / 2 + m_mapSize[1] / 2;
            float centerHight = std::abs(i) + std::abs(j);
            if (centerHight < 1)
                centerHight = 1;
            for (float k = 0; k < randomHight - centerHight; k++) {
                std::srand(std::rand() * std::time(nullptr));
                m_tilesDecor.push_back(
                    Tile {
                        .position = glm::vec3((static_cast<float>(i) * (m_tileSize[0] * 2 + m_tileSpacing[0])), m_tileHeight - m_tileSize[0] * 2 * static_cast<float>(k) + (0.001 * static_cast<float>(k)), (static_cast<float>(j) * (m_tileSize[1] * 2 + m_tileSpacing[1]))),
                        .rotation = glm::vec3(std::rand() % 4 * 90, std::rand() % 4 * 90, std::rand() % 4 * 90),
                        .mesh = m_tilesMeshes[static_cast<int>(i + j) % 2 == 0 ? "white" : "black"]
                    }
                );
            }
        }
    }
}

App::~App() {
    //close(m_socket);
}

void App::createScene() {
    m_scene->staticActors.clear();
    m_scene->animatedActors.clear();

    // Island tiles creation
    for (const auto& tile : m_tilesDecor) {
        m_scene->staticActors.push_back(IRenderer::StaticActor({
            .mesh = tile.mesh,
            .position = tile.position,
            .scale = m_tileSize,
            .rotation = tile.rotation,
            .color = glm::vec3(-glm::abs(tile.position[1]) / 100 + 1, -glm::abs(tile.position[1]) / 100 + 1, -glm::abs(tile.position[1]) / 100 + 1)
        }));
    }

    // Eggs creation
    for (const auto& egg : m_eggs) {
        m_scene->staticActors.push_back(IRenderer::StaticActor({
            .mesh = m_eggMesh,
            .position = egg.second.position,
            .scale = glm::vec3(0.1),
            .rotation = glm::vec3(270, 0, 0)
        }));
    }

    // Create all island tiles and ressources
    for (float i = -m_mapSize[0] / 2; i < m_mapSize[0] / 2; i++) {
        for (float j = -m_mapSize[1] / 2; j < m_mapSize[1] / 2; j++) {
            const TileContent& tile = m_map[i + m_mapSize[0] / 2][j + m_mapSize[1] / 2];

            // Display the ressources
            for (const auto& [ressourceType, offset] : m_ressourceOffset) {
                const glm::vec3 ressourcePosition = glm::vec3((static_cast<float>(i) * (m_tileSize[0] + m_tileSpacing[0])), m_resourceHeight, (static_cast<float>(j) * (m_tileSize[1] + m_tileSpacing[1])));
                glm::vec3 ressourceRotation = glm::vec3(0, m_ressourcesRotation, 0);
                m_ressourcesRotation += m_ressourcesRotationSpeed;
                if (m_ressourcesRotation > 360)
                    m_ressourcesRotation = 0;

                for (int nb = 0; nb < tile.ressources[ressourceType]; nb++)
                    m_scene->staticActors.push_back(
                        IRenderer::StaticActor({
                            m_ressourceMesh.at(ressourceType),
                            ressourcePosition + glm::vec3(static_cast<float>(nb % m_ressourceLayer) * offset[0] - m_tileSize[2] / 2 + m_tileSize[2] * 1/8, glm::floor(static_cast<float>(nb) / static_cast<float>(m_ressourceLayer)) * offset[1], offset[2] - m_tileSize[2] / 2),
                            m_resourceSize,
                            ressourceRotation,
                            glm::vec3(1, 1, 1)
                        })
                    );
            }
        }
    }

    createPlayers();
    addBroadcasts();
}

/*
void App::connectToServer(int port) {
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0)
        throw std::runtime_error("Socket creation failed");

    sockaddr_in server_addr {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = { .s_addr = INADDR_ANY },
        .sin_zero = { 0 }
    };

    if (connect(m_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0)
        throw std::runtime_error("Connection failed");

    std::array<char, BUFFER_SIZE> buffer{};
    if (read(m_socket, buffer.data(), BUFFER_SIZE) < 0)
        throw std::runtime_error("Read failed");

    if (std::string(buffer.data()) != "WELCOME\n")
        throw std::runtime_error("Connection failed");

    if (write(m_socket, "GRAPHIC\n", 8) < 0)
        throw std::runtime_error("Write failed");

    LOG("Connected to server", GREEN);
}
*/

void App::drawUi() noexcept {   // NOLINT
    // Mesh selection
    ImGui::Begin("Mesh and Animation Selection");
    for (auto& [teamName, team] : m_teams) {
        ImGui::TextColored(ImVec4(team.teamColor[0], team.teamColor[1], team.teamColor[2], 1), "%s", teamName.c_str());
        const char* currentMesh = team.mesh.first.c_str();
        if (ImGui::BeginCombo(("Mesh##" + teamName).c_str(), currentMesh)) {
            for (auto& [playerName, playerMesh] : m_playerMeshes) {
                bool isSelected = (team.mesh.second == playerMesh.first);
                if (ImGui::Selectable(playerName.c_str(), isSelected)) {
                    team.mesh.second = playerMesh.first;
                    team.mesh.first = playerName;
                    updatePlayersAnim();
                    createScene();
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    ImGui::End();

    ImGui::Begin("Trantor");
    for (std::size_t i = 0; i < RESNUMBER; ++i)
    {
        ImGui::Image(reinterpret_cast<ImTextureID>(m_resIcons[i]), {60, 60}, ImVec2(0, 1), ImVec2(1, 0));   // NOLINT
        ImGui::SameLine();
        ImGui::Text("%d\n", _mapInventory.ressources[i]);
    }
    ImGui::End();


    // Sort players by level
    std::vector<std::pair<int, Player>> players;
    for (auto& [id, player] : m_players)
        players.push_back({id, player});

    std::sort(players.begin(), players.end(), [](const auto& a, const auto& b) {
        return a.second.level > b.second.level;
    });

    ImGui::Begin("Trantorians");
    for (auto& [teamName, teamInfo] : m_teams)
    {
        if (ImGui::CollapsingHeader(teamName.c_str()))
        {
            for (auto& [id, player] : players)
            {
                if (player.teamName == teamName)
                {
                    ImGui::Text("Trantorian id: %d, level: %d\n", id, player.level);
                    if (ImGui::TreeNode((std::to_string(id) + " Inventory").c_str()))
                    {
                        for (std::size_t i = 0; i < RESNUMBER; ++i)
                        {
                            ImGui::Image(reinterpret_cast<ImTextureID>(m_resIcons[i]), {60, 60}, ImVec2(0, 1), ImVec2(1, 0));   // NOLINT
                            ImGui::SameLine();
                            ImGui::Text("%d", player.inv.ressources[i]);
                        }

                        ImGui::TreePop();
                    }
                    ImGui::Separator();
                }
            }
        }
    }
    ImGui::End();

    static int freq{};
    ImGui::Begin("Parameters");
    ImGui::InputInt("Frequency", &freq);
    freq = std::clamp(freq, 1, 1000);
    if (ImGui::Button("Send request"))
    {
        dprintf(_networkManager.getSocket(), "sst %d\n", freq);
        m_speed = freq;
    }
    ImGui::End();

    // Add logs to the UI
    ImGui::Begin("Logs");
    for (const auto& log : m_logs) {
        // Make the time a string
        std::array<char, 9> timeStr{};
        strftime(timeStr.data(), 9, "%H:%M:%S", localtime(&log.getTime()));

        // Display the log
        ImGui::TextColored(log.getColor(), "[%s] %s", timeStr.data(), log.getMessage().c_str());
        ImGui::SetScrollHereY(1);   // To scroll to the bottom
    }
    ImGui::End();
}

void App::createPlayers() {
    m_scene->animatedActors.clear();
    for (auto& [playerNumber, player] : m_players) {
        int newOrientation = 1;
        if (player.orientation == 1)
            newOrientation = 3;
        else if (player.orientation == 2)
            newOrientation = 2;
        else if (player.orientation == 3)
            newOrientation = 1;
        else if (player.orientation == 4)
            newOrientation = 0;
        const glm::vec3 playerRotation = glm::vec3(0, (newOrientation - 1) * 90, 0);

        m_scene->animatedActors.push_back({
            m_teams[player.teamName].mesh.second,
            player.animator,
            player.position + player.visualPositionOffset,
            m_playerMeshes[m_teams[player.teamName].mesh.first].second,
            playerRotation,
            glm::vec3(1, 1, 1)
        });
        m_scene->staticActors.push_back({
            .mesh = m_teamIndicatorMesh,
            .position = player.position + player.visualPositionOffset + glm::vec3(0, 3.5, 0),
            .scale = glm::vec3(1, 1, 1),
            .rotation = glm::vec3(0, 0, 0),
            .color = m_teams[player.teamName].teamColor
        });
    }
}

void App::addBroadcasts() {
    for (std::size_t i = 0; i < m_broadcasts.size(); i++) {
        std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
        float elapsedTime = std::chrono::duration<float, std::milli>(currentTime - m_broadcasts[i].startTime).count();

        if (elapsedTime > 2000)
            m_broadcasts.erase(m_broadcasts.begin() + static_cast<int>(i));
        else {
            m_scene->staticActors.push_back({
                m_broadcastMesh,
                glm::vec3(m_broadcasts[i].position) + glm::vec3(0, 0.5, 0),
                glm::vec3(elapsedTime / 50, 1.5, elapsedTime / 50),
                glm::vec3(0, 0, 0)
            });
        }
    }
}

void App::run() {
    // Load imgui settings
    ImGui::LoadIniSettingsFromDisk("../imgui.ini");

    // Buffer to store the data received from the server
    //std::array<char, BUFFER_SIZE> buffer{};
    std::string serverData{};

    while (!m_renderer->shouldStop()) {
        m_startFrameTime = std::chrono::high_resolution_clock::now();
        // Check if there is data to read from the server
        if (_protocolHandler.readDataFromServer(serverData))
        {
            const std::string &bufferView(serverData);
            updateMap(bufferView);
            updateEggs(bufferView);
            updatePlayers(bufferView);
        }
        /*
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(m_socket, &readfds);
        timeval timeout { .tv_sec = 0, .tv_usec = 0 };
        if (select(m_socket + 1, &readfds, nullptr, nullptr, &timeout) > 0) {
            // Read the data from the server
            buffer.fill(0);
            size_t readSize = 0;
            while (readSize < BUFFER_SIZE) {
                readSize += read(m_socket, buffer.data() + readSize, BUFFER_SIZE - readSize);
                if (buffer[readSize - 1] == '\n')
                    break;
            }

            const std::string& bufferView(buffer.data());

            // Update the map and players
            updateMap(bufferView);
            updateEggs(bufferView);
            updatePlayers(bufferView);
        }
        */

        // Begin UI rendering
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport();

        // Render the scene
        drawUi();
        updatePlayersAnim();
        createScene();
        m_renderer->render(m_scene, static_cast<float>(m_speed));
        m_endFrameTime = std::chrono::high_resolution_clock::now();
        m_frameTime = std::chrono::duration<float, std::milli>(m_endFrameTime - m_startFrameTime).count();
    }

    // TODO: remove this
    // ImGui::SaveIniSettingsToDisk("../imgui.ini");
}
