#include "pch.h"

std::unordered_map<xuid_t, unsigned short> thirstyList;
std::unordered_map<xuid_t, unsigned short> thirstyTime;
std::unordered_map<xuid_t, bool> isNether;

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

void onPlayerUseItem(PlayerUseItemEvent& ev) { //水瓶
	ItemStack item = ev.getPlayer().get().getCarriedItem();
	std::string itemName;
	if (item.getId() != 0) {
		SymCall("?getSerializedName@Item@@QEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ",
			void, const Item*, std::string*)(item.getItem(), &itemName);
	}
	else {
		itemName = "air";
	}
	if (itemName == "minecraft:potion") {
		if (item.getAuxValue() != 0) {
			ev.getPlayer().sendText(u8"非纯净水不能补充TP", JUKEBOX_POPUP);
			return;
		}
		xuid_t plXuid = ev.getPlayer().getXuid();
		if (BDX::runcmdEx("clear \"" + ev.getPlayer().getRealName() + "\" potion 0 1").first) {
			BDX::runcmdEx("give \"" + ev.getPlayer().getRealName() + "\" glass_bottle 1");
			ev.getPlayer().sendText(u8"§b你打开瓶子喝了一口水，TP+" + std::to_string(addThirst(plXuid, 20)));;
		}
		else {
			ev.getPlayer().sendText(u8"§b水撒地上了");
		}
	}
}

void onPlayerDestroy(PlayerDestroyEvent& ev) { //水桶及空手喝水
	ItemStack item = ev.getPlayer().get().getCarriedItem();
	std::string itemName;
	if (item.getId() != 0) {
		SymCall("?getSerializedName@Item@@QEBA?AV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@XZ",
			void, const Item*, std::string*)(item.getItem(), &itemName);
	}
	else {
		itemName = "air";
	}

	//ev.getPlayer().sendText(itemName, JUKEBOX_POPUP);
	if (itemName == "minecraft:water_bucket") {
		if (BDX::runcmdEx("clear \"" + ev.getPlayer().getRealName() + "\" water_bucket 0 1").first) {
			BDX::runcmdEx("give \"" + ev.getPlayer().getRealName() + "\" bucket 1");
			//add
			xuid_t plXuid = ev.getPlayer().getXuid();
			ev.getPlayer().sendText(u8"§b你大口喝了一桶水，TP+" + std::to_string(addThirst(plXuid, 50)));
		}
		else {
			ev.getPlayer().sendText(u8"§b水撒地上了");
		}
		ev.setCancelled();
	}
	if (ev.getPlayer().get().isInWater() && itemName == "air") {
		xuid_t plXuid = ev.getPlayer().getXuid();
		ev.getPlayer().sendText(u8"§b你将河水捧在手上，一口气喝了下去，TP+" + std::to_string(addThirst(plXuid, 15)));
		ev.setCancelled();
	}
}

void onPlayerPreJoin(PlayerPreJoinEvent& ev) {
	xuid_t xuid = stoull(ExtendedCertificate::getXuid(ev.cert));
	if (thirstyList.find(xuid) == thirstyList.end()) {
		thirstyList[xuid] = 100;
	}
}

void onPlayerJoin(PlayerJoinEvent& ev) {
	xuid_t xuid = ev.getPlayer().getXuid();
	if (ev.getPlayer().getDimID() == 1) {
		isNether[xuid] = true;
	}
	else {
		isNether[xuid] = false;
	}
	thirstyTime[xuid] = 0;
}

void onPlayerDeath(PlayerDeathEvent& ev) {
	xuid_t plXuid = ev.getPlayer().getXuid();
	thirstyList[plXuid] = 100;
	thirstyTime[plXuid] = 0;
}

short ticks = 0;
THook(void, "?normalTick@Player@@UEAAXXZ", Player* player) {
	if (ticks > 0) {
		WPlayer wp = WPlayer(*player);
		//ItemStack* itemStack = SymCall("?getSelectedItem@Player@@QEBAAEBVItemStack@@XZ", ItemStack*, Player*)(player);
		//if (thirstyList[plXuid] != 0) thirstyTime[plXuid] = thirstyTime[plXuid] + 1;
		int thirsty = thirstyList[wp.getXuid()];
		std::string popup;
		if (thirsty <= 20) {
			if (thirsty == 0) player->setOnFire(1);
			//std::cout << SymCall("?getSpeed@Player@@UEBAMXZ", float, Player*)(player) << "\n";
			//SymCall("?setSpeed@Player@@UEAAXM@Z", void, Player*, float)(player, 0.05);
			BDX::runcmdEx("effect \"" + wp.getRealName() + "\" slowness 1");
			popup = u8"§cTP:" + std::to_string(thirsty) + u8"%%";
		}
		else {
			if (wp.getDimID() == 1) {
				popup = u8"§6TP:" + std::to_string(thirsty) + u8"%%";
			}
			else {
				popup = u8"§bTP: " + std::to_string(thirsty) + u8"%%";
			}
		}
		wp.sendText(popup, TIP);
		ticks = 0;
	}
	return original(player);
}

THook(void, "?eat@Player@@QEAAXHM@Z", Player* player, int a1, float a2) {
	//std::cout << player->getNameTag() << " " << a1 << " " << a2 << "\n";
	WPlayer wp = WPlayer(*player);
	wp.sendText(u8"§b从食物中摄取水分，TP+" + std::to_string(addThirst(wp.getXuid(), (a1 / 2))));
	return original(player, a1, a2);
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
			std::string popup;
			if (thirsty <= 20) {
				if (thirsty == 0) wp.get().setOnFire(2);
				BDX::runcmdEx("effect \"" + wp.getRealName() + "\" slowness 2");
				popup = u8"§cTP:" + std::to_string(thirsty) + u8"%%";
			}
			else {
				if (isNether[wp.getXuid()]) {
					popup = u8"§6TP:" + std::to_string(thirsty) + u8"%%";
				}
				else {
					popup = u8"§bTP: " + std::to_string(thirsty) + u8"%%";
				}
			}
			wp.sendText(popup, TIP);
		}
	}
}*/

void entry() {
	addListener(onPlayerDestroy);
	addListener(onPlayerUseItem);
	addListener(onPlayerDeath);
	addListener([](PlayerChangeDimEvent& ev) {
		if (ev.getPlayer().getDimID() == 1) {
			isNether[ev.getPlayer().getXuid()] = true;
			ev.getPlayer().sendText(u8"§b[URSystem] 地狱过于炎热，TP消耗速度是主世界的两倍");
		}
		else {
			isNether[ev.getPlayer().getXuid()] = false;
		}
		});
	addListener(onPlayerJoin);
	addListener(onPlayerPreJoin);
	addListener([](PlayerLeftEvent& ev) {
		thirstyTime.erase(ev.getPlayer().getXuid());
		});
	std::thread t(timer);
	t.detach();
	std::cout << "[ThirstPoint] loaded!\n";
}