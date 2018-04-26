/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2013 Peter Powell <petpow@saberuk.com>
 *
 * This file is part of InspIRCd.  InspIRCd is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/* $ModAuthor: Peter "SaberUK" Powell, Modified by Sebastian Nielsen */
/* $ModDesc: Allows localhost to forcibly oper another user. */
/* $ModDepends: core 2.0 */

#include "inspircd.h"

class CommandSVSOper : public Command
{
 public:
	CommandSVSOper(Module* Creator)
		: Command(Creator, "SVSOPER", 3, 3)
	{
		// Anyone can run the command, even non-opers, but only if they connect from localhost. This allows services bots to oper up themselves
		// even if the /OPER command is disabled in server configuration.
		this->syntax = "<nick> <oper account> <ip adress>";
	}

	CmdResult Handle(const std::vector<std::string>& parameters, User* user)
	{
		User* target = ServerInstance->FindNick(parameters[0]);
		if (!target)
			return CMD_FAILURE;

		// Command can only be ran from localhost.
		if (strcmp(user->GetIPString(),"127.0.0.1") != 0)
		{
			user->WriteNumeric(ERR_NOPRIVILEGES, "SVSOPER can ONLY be run from Localhost");
			return CMD_FAILURE;
		}
		if (IS_LOCAL(target))
		{
			// I hope whoever came up with the idea to store types like this dies in a fire.
			OperIndex::iterator iter = ServerInstance->Config->oper_blocks.find(" " + parameters[1]);
			if (iter == ServerInstance->Config->oper_blocks.end())
				return CMD_FAILURE;

			// Check that the IP specified in command matches the real IP of the user. This prevents a race condition during
			// a netmerge where someone takes over a oper's nick while the SVSOPER command traverses the network.
			// If the IP does not match intended IP, command is ignored.
			if (target->GetIPString() != parameters[2])
				return CMD_FAILURE;
			target->Oper(iter->second);
		}
		return CMD_SUCCESS;
	}

	RouteDescriptor GetRouting(User* user, const std::vector<std::string>& parameters)
	{
		User* target = ServerInstance->FindNick(parameters[0]);
		if (!target)
			return ROUTE_LOCALONLY;
		return ROUTE_OPT_UCAST(target->server);
	}
};

class ModuleSVSOper : public Module
{
	CommandSVSOper command;

 public:
	ModuleSVSOper()
		: command(this)
	{
	}

	void init()
	{
		ServerInstance->Modules->AddService(command);
	}

	Version GetVersion()
	{
		return Version("Allows localhost to forcibly oper another user.", VF_OPTCOMMON);
	}
};

MODULE_INIT(ModuleSVSOper)
