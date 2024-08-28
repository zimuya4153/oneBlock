#pragma once

#include "Entry.h"

#include <ll/api/Logger.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/mod/RegisterHelper.h>
#include <ll/api/service/Bedrock.h>
#include <mc/enums/BlockUpdateFlag.h>
#include <mc/locale/I18n.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/registry/BlockTypeRegistry.h>
#include <mc/world/level/dimension/VanillaDimensions.h>
#include <mc/world/level/levelgen/GeneratorType.h>
#include <memory>
#include <more_dimensions/api/dimension/CustomDimensionManager.h>
#include <more_dimensions/api/dimension/SimpleCustomDimension.h>

ll::Logger logger("oneBlock");

int randomInt(int max) {
    static std::random_device                 rd;
    static std::default_random_engine         e(rd());
    static std::uniform_int_distribution<int> dist(0, max);
    return dist(e);
}

void updateBlock() {
    try {
        auto& source = ll::service::getLevel()
                           ->getDimension(VanillaDimensions::fromString("one_block"))
                           ->getBlockSourceFromMainChunkSource();
        if (source.getBlock(0, 0, 0).isAir()) {
            logger.debug("检测方块为空气，开始放置");
            auto block          = Block::tryGetFromRegistry("minecraft:air");
            auto blockRegistrys = BlockTypeRegistry::mBlockComplexAliasPostSplitBlockNamesList;
            int  count          = 0;
            do {
                try {
                    if (count > 30) return logger.error("获取方块注册重试超过30次失败，已停止获取。");
                    count++;
                    auto& blockRegistry = blockRegistrys.at(randomInt(static_cast<int>(blockRegistrys.size())));
                    block               = Block::tryGetFromRegistry(
                        blockRegistry.at(randomInt(static_cast<int>(blockRegistry.size()))).get()
                    );
                } catch (...) {}
            } while (!block.has_value() || block->isUnbreakable() || block->isAir()
                     || block->buildDescriptionId() == getI18n().get(block->buildDescriptionId(), {}));
            logger.debug("放置方块:{}", block->getTypeName());
            source.setBlock(BlockPos(0, 0, 0), block, static_cast<int>(BlockUpdateFlag::All), nullptr, nullptr);
        }
    } catch (...) {}
}

namespace oneBlock {

static std::unique_ptr<Entry> instance;

Entry& Entry::getInstance() { return *instance; }

bool Entry::load() { return true; }

bool Entry::enable() {
    more_dimensions::CustomDimensionManager::getInstance()
        .addDimension<more_dimensions::SimpleCustomDimension>("one_block", 0, GeneratorType::Void);
    return true;
}

bool Entry::disable() { return true; }

bool Entry::unload() { return true; }

} // namespace oneBlock

LL_REGISTER_MOD(oneBlock::Entry, oneBlock::instance);

LL_AUTO_TYPE_INSTANCE_HOOK(
    BlockChangedEventHook,
    HookPriority::Normal,
    BlockSource,
    &BlockSource::_blockChanged,
    void,
    BlockPos const&              pos,
    uint                         layer,
    Block const&                 block,
    Block const&                 previousBlock,
    int                          updateFlags,
    ActorBlockSyncMessage const* syncMsg,
    Actor*                       blockChangeSource
) {
    origin(pos, layer, block, previousBlock, updateFlags, syncMsg, blockChangeSource);
    updateBlock();
}

LL_TYPE_INSTANCE_HOOK(
    PlayerChangeDimensionHook,
    HookPriority::Normal,
    Level,
    &Level::requestPlayerChangeDimension,
    void,
    Player&                  player,
    ChangeDimensionRequest&& changeRequest
) {
    origin(player, std::move(changeRequest));
    updateBlock();
}