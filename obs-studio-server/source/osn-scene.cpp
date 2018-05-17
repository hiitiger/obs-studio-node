#include "osn-scene.hpp"
#include "osn-sceneitem.hpp"
#include "error.hpp"
#include <list>
#include "shared.hpp"

void osn::Scene::Register(ipc::server& srv) {
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("Scene");
	cls->register_function(std::make_shared<ipc::function>("Create", std::vector<ipc::type>{ipc::type::String}, Create));
	cls->register_function(std::make_shared<ipc::function>("CreatePrivate", std::vector<ipc::type>{ipc::type::String}, CreatePrivate));
	cls->register_function(std::make_shared<ipc::function>("FromName", std::vector<ipc::type>{ipc::type::String}, FromName));

	cls->register_function(std::make_shared<ipc::function>("Release", std::vector<ipc::type>{ipc::type::UInt64}, Release));
	cls->register_function(std::make_shared<ipc::function>("Remove", std::vector<ipc::type>{ipc::type::UInt64}, Remove));

	cls->register_function(std::make_shared<ipc::function>("AsSource", std::vector<ipc::type>{ipc::type::UInt64}, AsSource));
	cls->register_function(std::make_shared<ipc::function>("Duplicate", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String, ipc::type::Int32}, Duplicate));

	cls->register_function(std::make_shared<ipc::function>("AddSource", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64}, AddSource));
	cls->register_function(std::make_shared<ipc::function>("FindItem", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String}, FindItemByName));
	cls->register_function(std::make_shared<ipc::function>("FindItem", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Int64}, FindItemByItemId));
	cls->register_function(std::make_shared<ipc::function>("MoveItem", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Int32, ipc::type::Int32}, MoveItem));
	cls->register_function(std::make_shared<ipc::function>("GetItem", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Int32}, GetItem));
	cls->register_function(std::make_shared<ipc::function>("GetItems", std::vector<ipc::type>{ipc::type::UInt64}, GetItems));
	cls->register_function(std::make_shared<ipc::function>("GetItemsInRange", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Int32, ipc::type::Int32}, GetItemsInRange));

	cls->register_function(std::make_shared<ipc::function>("Connect", std::vector<ipc::type>{ipc::type::UInt64}, Connect));
	cls->register_function(std::make_shared<ipc::function>("Disconnect", std::vector<ipc::type>{ipc::type::UInt64}, Disconnect));
	srv.register_collection(cls);
}

void osn::Scene::Create(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_scene_t* scene = obs_scene_create(args[0].value_str.c_str());
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to create scene."));
		AUTO_DEBUG;
		return;
	}

	obs_source_t* source = obs_scene_get_source(scene);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to get source from scene."));
		AUTO_DEBUG;
		return;
	}

	uint64_t uid = osn::Source::GetInstance()->Allocate(source);
	if (uid == UINT64_MAX) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Index list is full."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::Scene::CreatePrivate(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_scene_t* scene = obs_scene_create_private(args[0].value_str.c_str());
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to create scene."));
		AUTO_DEBUG;
		return;
	}

	obs_source_t* source = obs_scene_get_source(scene);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to get source from scene."));
		AUTO_DEBUG;
		return;
	}

	uint64_t uid = osn::Source::GetInstance()->Allocate(source);
	if (uid == UINT64_MAX) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Index list is full."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::Scene::FromName(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = obs_get_source_by_name(args[0].value_str.c_str());
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to get source from scene."));
		AUTO_DEBUG;
		return;
	}

	uint64_t uid = osn::Source::GetInstance()->Get(source);
	if (uid == UINT64_MAX) {
		// This is an impossible case, but we handle it in case it happens.
		obs_source_release(source);
	#ifdef DEBUG // Debug should throw an error for debuggers to catch.
		throw std::runtime_error("Source found but not indexed.");
	#endif
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Source found but not indexed."));
		AUTO_DEBUG;
		return;
	}

	obs_source_release(source);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::Scene::Release(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not a scene."));
		AUTO_DEBUG;
		return;
	}

	std::list<obs_sceneitem_t*> items;
	auto cb = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
		std::list<obs_sceneitem_t*>* items = reinterpret_cast<std::list<obs_sceneitem_t*>*>(data);
		obs_sceneitem_addref(item);
		items->push_back(item);
		return true;
	};
	obs_scene_enum_items(scene, cb, &items);

	obs_source_release(source);
	if (obs_source_removed(source)) {
		osn::Source::GetInstance()->Free(args[0].value_union.ui64);
		for (auto item : items) {
			osn::SceneItem::Manager::GetInstance().free(item);
			obs_sceneitem_release(item);
		}
	}

	for (auto item : items) {
		obs_sceneitem_release(item);
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::Scene::Remove(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not a scene."));
		AUTO_DEBUG;
		return;
	}

	std::list<obs_sceneitem_t*> items;
	auto cb = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
		std::list<obs_sceneitem_t*>* items = reinterpret_cast<std::list<obs_sceneitem_t*>*>(data);
		obs_sceneitem_addref(item);
		items->push_back(item);
		return true;
	};
	obs_scene_enum_items(scene, cb, &items);

	obs_source_remove(source);
	osn::Source::GetInstance()->Free(args[0].value_union.ui64);

	for (auto item : items) {
		osn::SceneItem::Manager::GetInstance().free(item);
		obs_sceneitem_release(item);
		obs_sceneitem_release(item);
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::Scene::AsSource(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Scenes are stored as such.
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(args[0].value_union.ui64));
	AUTO_DEBUG;
}

void osn::Scene::Duplicate(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not a scene."));
		AUTO_DEBUG;
		return;
	}

	obs_scene_t* scene2 = obs_scene_duplicate(scene, args[1].value_str.c_str(), (obs_scene_duplicate_type)args[2].value_union.i32);
	if (!scene2) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to duplicate scene."));
		AUTO_DEBUG;
		return;
	}

	obs_source_t* source2 = obs_scene_get_source(scene2);
	if (!source2) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to get source from duplicate scene."));
		AUTO_DEBUG;
		return;
	}

	uint64_t uid = osn::Source::GetInstance()->Allocate(source2);
	if (uid == UINT64_MAX) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Index list is full."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::Scene::AddSource(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not a scene."));
		AUTO_DEBUG;
		return;
	}

	obs_source_t* added_source = osn::Source::GetInstance()->Get(args[1].value_union.ui64);
	if (!added_source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference to add is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_sceneitem_t* item = obs_scene_add(scene, added_source);

	utility::unique_id::id_t uid = osn::SceneItem::Manager::GetInstance().allocate(item);
	if (uid == UINT64_MAX) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Index list is full."));
		AUTO_DEBUG;
		return;
	}
	obs_sceneitem_addref(item);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value((uint64_t)uid));
	AUTO_DEBUG;
}

void osn::Scene::FindItemByName(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not a scene."));
		AUTO_DEBUG;
		return;
	}

	obs_sceneitem_t* item = obs_scene_find_source(scene, args[1].value_str.c_str());
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Source not found."));
		AUTO_DEBUG;
		return;
	}

	utility::unique_id::id_t uid = osn::SceneItem::Manager::GetInstance().find(item);
	if (uid == UINT64_MAX) {
		uid = osn::SceneItem::Manager::GetInstance().allocate(item);
		if (uid == UINT64_MAX) {
			rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
			rval.push_back(ipc::value("Index list is full."));
			AUTO_DEBUG;
			return;
		}
		obs_sceneitem_addref(item);
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value((uint64_t)uid));
	AUTO_DEBUG;
}

void osn::Scene::FindItemByItemId(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not a scene."));
		AUTO_DEBUG;
		return;
	}

	obs_sceneitem_t* item = obs_scene_find_sceneitem_by_id(scene, args[1].value_union.i64);
	if (!item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Source not found."));
		AUTO_DEBUG;
		return;
	}

	utility::unique_id::id_t uid = osn::SceneItem::Manager::GetInstance().find(item);
	if (uid == UINT64_MAX) {
		uid = osn::SceneItem::Manager::GetInstance().allocate(item);
		if (uid == UINT64_MAX) {
			rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
			rval.push_back(ipc::value("Index list is full."));
			AUTO_DEBUG;
			return;
		}
		obs_sceneitem_addref(item);
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value((uint64_t)uid));
	AUTO_DEBUG;
}

void osn::Scene::MoveItem(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not a scene."));
		AUTO_DEBUG;
		return;
	}

	// Listen up! This function does not work as you might expect it to (lowest index is furthest
	//  back), instead it works on the inverted order, so the lowest index is the furthest in front.
	// While this may be weird at first, this does have some advantages as you do not have to guess 
	//  on what the best index for moving something in front is. Downside is that it is kinda weird
	//  to deal with when you're used to things being the other way around.
	
	size_t num_items = 0;
	auto cb_count = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
		(*(reinterpret_cast<size_t*>(data)))++;
		return true;
	};
	obs_scene_enum_items(scene, cb_count, &num_items);
	
	struct EnumData {
		obs_sceneitem_t* item = nullptr;
		int32_t findindex = 0;
		int32_t index = 0;
	} ed;
	ed.findindex = (num_items - 1) - args[1].value_union.i32;

	auto cb = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
		EnumData* items = reinterpret_cast<EnumData*>(data);
		if (items->index == items->findindex) {
			items->item = item;
			return false;
		}
		items->index++;
		return true;
	};
	obs_scene_enum_items(scene, cb, &ed);

	if (!ed.item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::OutOfBounds));
		rval.push_back(ipc::value("Index not found in Scene."));
		AUTO_DEBUG;
		return;
	}

	obs_sceneitem_set_order_position(ed.item, (num_items - 1) - args[2].value_union.i32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::Scene::GetItem(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not a scene."));
		AUTO_DEBUG;
		return;
	}

	struct EnumData {
		obs_sceneitem_t* item = nullptr;
		size_t findindex = 0;
		size_t index = 0;
	} ed;
	ed.findindex = args[1].value_union.ui64;

	auto cb = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
		EnumData* items = reinterpret_cast<EnumData*>(data);
		if (items->index == items->findindex) {
			items->item = item;
			return false;
		}
		items->index++;
		return true;
	};
	obs_scene_enum_items(scene, cb, &ed);

	if (!ed.item) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::OutOfBounds));
		rval.push_back(ipc::value("Index not found in Scene."));
		AUTO_DEBUG;
		return;
	}

	utility::unique_id::id_t uid = osn::SceneItem::Manager::GetInstance().find(ed.item);
	if (uid == UINT64_MAX) {
		uid = osn::SceneItem::Manager::GetInstance().allocate(ed.item);
		if (uid == UINT64_MAX) {
			rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
			rval.push_back(ipc::value("Index list is full."));
			AUTO_DEBUG;
			return;
		}
		obs_sceneitem_addref(ed.item);
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value((uint64_t)uid));
	AUTO_DEBUG;
}

void osn::Scene::GetItems(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not a scene."));
		AUTO_DEBUG;
		return;
	}

	std::list<obs_sceneitem_t*> items;
	auto cb = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
		std::list<obs_sceneitem_t*>* items = reinterpret_cast<std::list<obs_sceneitem_t*>*>(data);
		items->push_back(item);
		return true;
	};
	obs_scene_enum_items(scene, cb, &items);
	
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	for (obs_sceneitem_t* item : items) {
		utility::unique_id::id_t uid = osn::SceneItem::Manager::GetInstance().find(item);
		if (uid == UINT64_MAX) {
			uid = osn::SceneItem::Manager::GetInstance().allocate(item);
			if (uid == UINT64_MAX) {
				rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
				rval.push_back(ipc::value("Index list is full."));
				AUTO_DEBUG;
				return;
			}
			obs_sceneitem_addref(item);
		}
		rval.push_back(ipc::value((uint64_t)uid));
	}
	AUTO_DEBUG;
}

void osn::Scene::GetItemsInRange(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not a scene."));
		AUTO_DEBUG;
		return;
	}

	struct EnumData {
		std::list<obs_sceneitem_t*> items;
		size_t index_from = 0, index_to = 0;
		size_t index = 0;
	} ed;
	ed.index_from = args[1].value_union.ui64;
	ed.index_to = args[1].value_union.ui64;

	auto cb = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
		EnumData* ed = reinterpret_cast<EnumData*>(data);
		if ((ed->index >= ed->index_from) && (ed->index <= ed->index_to)) {
			ed->items.push_back(item);
		} else if (ed->index > ed->index_to) {
			return false;
		}
		ed->index++;
		return true;
	};
	obs_scene_enum_items(scene, cb, &ed);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	for (obs_sceneitem_t* item : ed.items) {
		utility::unique_id::id_t uid = osn::SceneItem::Manager::GetInstance().find(item);
		if (uid == UINT64_MAX) {
			uid = osn::SceneItem::Manager::GetInstance().allocate(item);
			if (uid == UINT64_MAX) {
				rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
				rval.push_back(ipc::value("Index list is full."));
				AUTO_DEBUG;
				return;
			}
			obs_sceneitem_addref(item);
		}
		rval.push_back(ipc::value((uint64_t)uid));
	}
	AUTO_DEBUG;
}

void osn::Scene::Connect(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	AUTO_DEBUG;
	// !FIXME! Signals
}

void osn::Scene::Disconnect(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	AUTO_DEBUG;
	// !FIXME! Signals
}
