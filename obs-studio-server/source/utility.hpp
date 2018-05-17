// Server program for the OBS Studio node module.
// Copyright(C) 2017 Streamlabs (General Workings Inc)
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#pragma once
#include <inttypes.h>
#include <list>
#include <limits>
#include <map>
#include <memory>

#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#else
#define FORCE_INLINE __attribute__((always_inline))
#endif
#define force_inline FORCE_INLINE

namespace utility {
	class unique_id {
		public:
		typedef uint64_t id_t;
		typedef std::pair<id_t, id_t> range_t;

		public:
		unique_id();
		virtual ~unique_id();

		id_t allocate();
		void free(id_t);

		bool is_allocated(id_t);
		id_t count(bool count_free);

		protected:
		bool mark_used(id_t);
		void mark_used_range(id_t, id_t);
		bool mark_free(id_t);
		void mark_free_range(id_t, id_t);

		private:
		std::list<range_t> allocated;
	};

	template<typename T>
	class unique_object_manager {
		protected:
		utility::unique_id id_generator;
		std::map<utility::unique_id::id_t, T*> object_map;
		
		public:
		unique_object_manager() {}
		~unique_object_manager() {}

		utility::unique_id::id_t allocate(T* obj) {
			utility::unique_id::id_t uid = id_generator.allocate();
			if (uid == std::numeric_limits<utility::unique_id::id_t>::max()) {
				return uid;
			}
			object_map.insert_or_assign(uid, obj);
			return uid;
		}

		utility::unique_id::id_t find(T* obj) {
			for (auto kv : object_map) {
				if (kv.second == obj) {
					return kv.first;
				}
			}
			return std::numeric_limits<utility::unique_id::id_t>::max();
		}
		T* find(utility::unique_id::id_t id) {
			auto iter = object_map.find(id);
			if (iter != object_map.end()) {
				return iter->second;
			}
			return nullptr;
		}

		utility::unique_id::id_t free(T* obj) {
			utility::unique_id::id_t uid = std::numeric_limits<utility::unique_id::id_t>::max();
			for (auto kv : object_map) {
				if (kv.second == obj) {
					uid = kv.first;
					object_map.erase(kv.first);
					break;
				}
			}
			return uid;
		}
		T* free(utility::unique_id::id_t id) {
			auto iter = object_map.find(id);
			if (iter == object_map.end()) {
				return nullptr;
			}
			T* obj = iter->second;
			object_map.erase(iter);
			return obj;
		}
	};

	template<typename T>
	class generic_object_manager {
		protected:
		utility::unique_id id_generator;
		std::map<utility::unique_id::id_t, T> object_map;

		public:
		generic_object_manager() {}
		~generic_object_manager() {}

		utility::unique_id::id_t allocate(T obj) {
			utility::unique_id::id_t uid = id_generator.allocate();
			if (uid == std::numeric_limits<utility::unique_id::id_t>::max()) {
				return uid;
			}
			object_map.insert_or_assign(uid, obj);
			return uid;
		}

		utility::unique_id::id_t find(T obj) {
			for (auto kv : object_map) {
				if (kv.second == obj) {
					return kv.first;
				}
			}
			return std::numeric_limits<utility::unique_id::id_t>::max();
		}
		T find(utility::unique_id::id_t id) {
			auto iter = object_map.find(id);
			if (iter != object_map.end()) {
				return iter->second;
			}
			return nullptr;
		}

		utility::unique_id::id_t free(T obj) {
			utility::unique_id::id_t uid = std::numeric_limits<utility::unique_id::id_t>::max();
			for (auto kv : object_map) {
				if (kv.second == obj) {
					uid = kv.first;
					object_map.erase(kv.first);
					break;
				}
			}
			return uid;
		}
		T free(utility::unique_id::id_t id) {
			auto iter = object_map.find(id);
			if (iter == object_map.end()) {
				return nullptr;
			}
			T obj = iter->second;
			object_map.erase(iter);
			return obj;
		}
	};
}
