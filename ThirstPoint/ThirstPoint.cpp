#include "pch.h"

std::unordered_map<xuid_t, unsigned short> thirstyList;
std::unordered_map<xuid_t, unsigned short> thirstyTime;
std::unordered_map<xuid_t, bool> isNether;
Logger<stdio_commit> LOG(stdio_commit("[ThirstPoint] "));

unsigned short addThirst(xuid_t xuid, unsigned short thirsty) {
	unsigned short thirsty1 = thirstyList[xuid];
	if ((thirsty1 + thirsty) > 100) {
		thirstyList[xuid] = 100;
		return 100 - thirsty1;
	}
	thirstyList[xuid] = thirsty1 + thirsty;
	return thirsty;
}

unsigned short delThirst(xuid_t xuid, unsigned short thirsty) {
	short thirsty1 = thirstyList[xuid];
	if ((thirsty1 - thirsty) < 0) {
		thirstyList[xuid] = 0;
		return 100 - thirsty1;
	}
	thirstyList[xuid] = thirsty1 - thirsty;
	return thirsty;
}

void onPlayerUseItem(PlayerUseItemEV ev) { //ˮƿ
	bool isSneaking = SymCall("?isSneaking@Actor@@QEBA_NXZ", bool, Actor*)(ev.Player);
	if (!isSneaking) return;
	ItemStack item = ev.Player->getCarriedItem();
	WPlayer wp = WPlayer(*ev.Player);
	string itemName;
	if (item.getId() != 0) {
		SymCall("?getSerializedName@Item@@QEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ",
			void, const Item*, string*)(item.getItem(), &itemName);
	}
	else {
		itemName = "air";
	}
	if (itemName == "minecraft:potion") {
		if (item.getAuxValue() != 0) {
			wp.sendText(u8"�Ǵ���ˮ���ܲ���TP", JUKEBOX_POPUP);
			return;
		}
		xuid_t plXuid = offPlayer::getXUID(ev.Player);
		string playerReal = wp.getName();
		if (liteloader::runcmdEx("clear \"" + playerReal + "\" potion 0 1").first) {
			liteloader::runcmdEx("give \"" + playerReal + "\" glass_bottle 1");
			wp.sendText(u8"��b���ƿ�Ӻ���һ��ˮ��TP+" + std::to_string(addThirst(plXuid, 20)));;
		}
	}
}

THook(bool, "?playerWillDestroy@BlockLegacy@@UEBA_NAEAVPlayer@@AEBVBlockPos@@AEBVBlock@@@Z", BlockLegacy* _this, Player& pl, BlockPos& blkpos, Block& bl) {
	ItemStack item = pl.getCarriedItem();
	string itemName;
	if (item.getId() != 0) {
		SymCall("?getSerializedName@Item@@QEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ",
			void, const Item*, string*)(item.getItem(), &itemName);
	}
	else {
		itemName = "air";
	}

	//ev.getPlayer().sendText(itemName, JUKEBOX_POPUP);
	WPlayer wp = WPlayer(pl);
	string playerReal = wp.getName();
	if (itemName == "minecraft:water_bucket") {
		if (liteloader::runcmdEx("clear \"" + playerReal + "\" water_bucket 0 1").first) {
			liteloader::runcmdEx("give \"" + playerReal + "\" bucket 1");
			//add
			xuid_t plXuid = offPlayer::getXUID(&pl);
			wp.sendText(u8"��b���ں���һͰˮ��TP+" + std::to_string(addThirst(plXuid, 50)));
		}
		return false;
	}
	if (pl.isInWater() && itemName == "air") {
		xuid_t plXuid = offPlayer::getXUID(&pl);
		wp.sendText(u8"��b�㽫��ˮ�������ϣ�һ����������ȥ��TP+" + std::to_string(addThirst(plXuid, 15)));
		return false;
	}
	return original(_this, pl, blkpos, bl);
}

/*void onPlayerPreJoin(PlayerPreJoinEvent& ev) {
	xuid_t xuid = std::stoull(ExtendedCertificate::getXuid(ev.cert));
	if (thirstyList.find(xuid) == thirstyList.end()) {
		thirstyList[xuid] = 100;
	}
}*/

void onPlayerPreJoin(PreJoinEV ev) {
	xuid_t xuid = offPlayer::getXUIDByCert(ev.cert);
	if (thirstyList.find(xuid) == thirstyList.end()) {
		thirstyList[xuid] = 100;
	}
}

void onPlayerJoin(JoinEV ev) {
	xuid_t xuid = offPlayer::getXUID(ev.Player);
	if (ev.Player->getDimensionId() == 1) {
		isNether[xuid] = true;
	}
	else {
		isNether[xuid] = false;
	}
	thirstyTime[xuid] = 0;
}

void onPlayerDeath(PlayerDeathEV ev) {
	xuid_t plXuid = offPlayer::getXUID(ev.Player);
	thirstyList[plXuid] = 100;
	thirstyTime[plXuid] = 0;
}

short ticks = 0;
THook(void, "?normalTick@Player@@UEAAXXZ", Player* player) {
	if (ticks > 0) {
		WPlayer wp = WPlayer(*player);
		//ItemStack* itemStack = SymCall("?getSelectedItem@Player@@QEBAAEBVItemStack@@XZ", ItemStack*, Player*)(player);
		//if (thirstyList[plXuid] != 0) thirstyTime[plXuid] = thirstyTime[plXuid] + 1;
		int thirsty = thirstyList[offPlayer::getXUID(player)];
		string popup;
		if (thirsty <= 20) {
			if (thirsty == 0) player->setOnFire(1);
			//std::cout << SymCall("?getSpeed@Player@@UEBAMXZ", float, Player*)(player) << "\n";
			//SymCall("?setSpeed@Player@@UEAAXM@Z", void, Player*, float)(player, 0.05);
			liteloader::runcmdEx("effect \"" + offPlayer::getRealName(player) + "\" slowness 2");
			popup = u8"��cTP:" + std::to_string(thirsty) + u8"%%";
		}
		else {
			if (wp.getDimID() == 1) {
				popup = u8"��6TP:" + std::to_string(thirsty) + u8"%%";
			}
			else {
				popup = u8"��bTP: " + std::to_string(thirsty) + u8"%%";
			}
		}
		wp.sendText(popup, TIP);
		ticks = 0;
	}
	return original(player);
}

THook(void, "?eat@Player@@QEAAXAEBVItemStack@@@Z", Player* player, ItemStack* item) {
	//std::cout << player->getNameTag() << " " << a1 << " " << a2 << "\n";
	WPlayer wp = WPlayer(*player);
	wp.sendText(u8"��b��ʳ������ȡˮ�֣�TP+" + std::to_string(addThirst(offPlayer::getXUID(player), 2)));
	return original(player, item);
}

void timer() {
	for (; true; Sleep(1000)) {
		ticks++;
		for (auto i : thirstyTime) {
			//if (thirstyList.find(i.first) == thirstyList.end()) continue;
			xuid_t xuid = i.first;
			thirstyTime[xuid] = i.second + 1;
			if (thirstyTime[xuid] >= 10) {
				if (isNether[xuid]) {
					delThirst(xuid, 2);
					thirstyTime[xuid] = 0;
				}
				else {
					delThirst(xuid, 1);
					thirstyTime[xuid] = 0;
				}
			}
		}
	}
}

/*
void popupTimer() {
	for (; true; Sleep(1500)) {
		for (auto pl : players) {
			WPlayer wp = pl.second;
			if (!wp) continue;
			int thirsty = thirstyList[wp.getXuid()];
			string popup;
			if (thirsty <= 20) {
				if (thirsty == 0) wp.get().setOnFire(2);
				liteloader::runcmdEx("effect \"" + wp.getRealName() + "\" slowness 2");
				popup = u8"��cTP:" + std::to_string(thirsty) + u8"%%";
			}
			else {
				if (isNether[wp.getXuid()]) {
					popup = u8"��6TP:" + std::to_string(thirsty) + u8"%%";
				}
				else {
					popup = u8"��bTP: " + std::to_string(thirsty) + u8"%%";
				}
			}
			wp.sendText(popup, TIP);
		}
	}
}*/

void entry() {
	Event::addEventListener(onPlayerUseItem);
	Event::addEventListener(onPlayerDeath);
	Event::addEventListener([](ChangeDimEV ev) {
		xuid_t xuid = offPlayer::getXUID(ev.Player);
		if (ev.Player->getDimensionId() == 1) {
			isNether[xuid] = true;
		}
		else {
			isNether[xuid] = false;
		}
		});
	Event::addEventListener(onPlayerJoin);
	Event::addEventListener(onPlayerPreJoin);
	Event::addEventListener([](LeftEV ev) {
		thirstyTime.erase(ev.xuid);
		});
	std::thread t(timer);
	t.detach();
	LOG("Loaded, version: 210704");
}