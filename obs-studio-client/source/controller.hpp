// Client module for the OBS Studio node module.
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
#include <memory>
#include <string>
#include "ipc-client.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

class Controller {
#pragma region Singleton
	public:
	static Controller& GetInstance() {
		static Controller _inst;
		return _inst;
	}

	private:
	Controller();
	~Controller();

	public: // C++11
	Controller(Controller const&) = delete;
	void operator=(Controller const&) = delete;
#pragma endregion Singleton
	
	public:
	std::shared_ptr<ipc::client> host(std::string uri);
	std::shared_ptr<ipc::client> connect(std::string uri);
	void disconnect();

	std::shared_ptr<ipc::client> GetConnection();

	private:
	bool m_isServer = false;
	std::shared_ptr<ipc::client> m_connection;
	uint64_t procId = 0;
};
