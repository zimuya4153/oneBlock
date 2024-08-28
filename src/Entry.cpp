#pragma once

#include "Entry.h"
#include "Config.h"

#include <ll/api/Config.h>
#include <ll/api/Logger.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/mod/RegisterHelper.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/service/ServerInfo.h>
#include <mc/enums/BlockUpdateFlag.h>
#include <mc/locale/I18n.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/block/Block.h>
#include <mc/world/level/block/registry/BlockTypeRegistry.h>
#include <mc/world/level/block/utils/BedrockBlockNames.h>
#include <mc/world/level/dimension/VanillaDimensions.h>
#include <mc/world/level/levelgen/GeneratorType.h>
#include <mc/world/level/material/Material.h>
#include <memory>
#include <more_dimensions/api/dimension/CustomDimensionManager.h>
#include <more_dimensions/api/dimension/SimpleCustomDimension.h>

ll::Logger       logger("oneBlock");
oneBlock::Config config;

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
        if (!source.getBlock(0, 0, 0).isAir()) return;
        logger.debug("检测方块为空气，开始放置");
        auto  block          = Block::tryGetFromRegistry(BedrockBlockNames::Air);
        auto& blockRegistrys = BlockTypeRegistry::mBlockLookupMap;
        int   count          = 0;
        do {
            try {
                if (count > 30) return logger.error("获取方块注册重试超过30次失败，已停止获取。");
                count++;
                auto it = blockRegistrys.begin();
                std::advance(it, randomInt(static_cast<int>(blockRegistrys.size()) - 1));
                logger.debug("尝试获取方块:{}", std::string(it->first));
                block = Block::tryGetFromRegistry(it->first);
            } catch (...) {}
        } while (!block.has_value() || block->isAir() || block->isEmpty() || block->isUnbreakable()
                 || block->getMaterial().isType(MaterialType::Deny) || block->getMaterial().isType(MaterialType::Allow)
                 || block->getMaterial().isType(MaterialType::Barrier)
                 || block->getMaterial().isType(MaterialType::StructureVoid)
                 || block->buildDescriptionId() == getI18n().get(block->buildDescriptionId(), {})
                 || std::count(config.blacklist.begin(), config.blacklist.end(), block->getTypeName()) > 0);
        logger.debug("放置方块:{}", block->getTypeName());
        source.setBlock(BlockPos(0, 0, 0), block.value(), static_cast<int>(BlockUpdateFlag::All), nullptr, nullptr);
    } catch (...) {}
}

namespace oneBlock {

static std::unique_ptr<Entry> instance;

Entry& Entry::getInstance() { return *instance; }

bool Entry::load() {
    auto path = getSelf().getConfigDir() / "config.json";
    try {
        if (!ll::config::loadConfig(config, path)) {
            ll::config::saveConfig(config, path);
        }
    } catch (...) {
        ll::config::saveConfig(config, path);
    }
    if (ll::getServerStatus() != ll::ServerStatus::Starting) {
        logger.error("禁止热加载！");
        return false;
    }
    return true;
}

bool Entry::enable() {
    if (ll::getServerStatus() != ll::ServerStatus::Starting) {
        logger.error("禁止热加载！");
        return false;
    }
    more_dimensions::CustomDimensionManager::getInstance()
        .addDimension<more_dimensions::SimpleCustomDimension>("one_block", 0, GeneratorType::Void);
    logger.debug("共发现注册 {} 个方块", std::to_string(BlockTypeRegistry::mBlockLookupMap.size()));
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

LL_AUTO_INSTANCE_HOOK(
    ReloadCommandExecuteHook,
    HookPriority::Normal,
    "?execute@ReloadCommand@@UEBAXAEBVCommandOrigin@@AEAVCommandOutput@@@Z",
    void,
    CommandOrigin const& commandOrigin,
    CommandOutput&       output
) {
    auto path = oneBlock::Entry::getInstance().getSelf().getConfigDir() / "config.json";
    try {
        if (!ll::config::loadConfig(config, path)) {
            ll::config::saveConfig(config, path);
        }
    } catch (...) {
        ll::config::saveConfig(config, path);
    }
    output.success("已尝试重载oneBlock配置文件");
    origin(commandOrigin, output);
}