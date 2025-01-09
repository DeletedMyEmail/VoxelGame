#pragma once

#include "Chunk.h"
#include "Player.h"

class GameWorld {
public:
    GameWorld(glm::vec3 playerPos, unsigned int chunksPerSide);

    void update();

    std::vector<Chunk>& getChunks() { return m_Chunks; }
    Player& getPlayer() { return m_Player; }
private:
    void playerCollisions();
    void initChunks(unsigned int chunksPerSide);
private:
    Player m_Player;
    std::vector<Chunk> m_Chunks;
};
