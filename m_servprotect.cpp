/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 * Copyright (C) 2009 Daniel De Graaf <danieldg@inspircd.org>
 * Copyright (C) 2008 Craig Edwards <craigedwards@brainbox.cc>
 * Copyright (C) 2007 Robin Burchell <robin+git@viroteck.net>
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
#include "inspircd.h"
/* $ModDesc: Provides config to protect ircops from kicks and mode changes. */


class ModuleServProtect : public Module {
 public:
        void init()
        {
                Implementation eventlist[] = { I_OnWhois, I_OnRawMode, I_OnUserPreKick };
                ServerInstance->Modules->Attach(eventlist, this, sizeof(eventlist)/sizeof(Implementation));
        }
        Version GetVersion()
        {
                return Version("Provides config to protect ircops from kicks and mode changes.");
        }
        void OnWhois(User* src, User* dst)
        {
                if (IS_OPER(dst))
                {
                        ConfigTag* qtag = dst->oper->type_block;
                        if (qtag->getString("immunity") == "yes")
                        {
                                ServerInstance->SendWhoisLine(src, dst, 310, src->nick+" "+dst->nick+" :is an "+ServerInstance->Config->Network+" Super IRCO$
                        }
                }
        }
        ModResult OnRawMode(User* user, Channel* chan, const char mode, const std::string &param, bool adding, int pcnt)
        {
                if (!adding && chan && IS_LOCAL(user) && !param.empty() && !ServerInstance->ULine(user->server))
                {
                        User *u = ServerInstance->FindNick(param);
                        if (u)
                        {
                                if (IS_OPER(u))
                                {
                                        Membership* memb = chan->GetUser(u);
                                        ConfigTag* utag = u->oper->type_block;
                                        if (utag->getString("immunity") == "yes" && memb && memb->modes.find(mode) != std::string::npos && u != user)
                                        {
                                                user->WriteNumeric(482, "%s %s :You are not permitted to remove privileges from %s Super IRCOPs", user->nick$
                                                return MOD_RES_DENY;
                                        }
                                }
                        }
                }
                return MOD_RES_PASSTHRU;
        }
        ModResult OnUserPreKick(User *src, Membership* memb, const std::string &reason)
        {
                if (IS_OPER(memb->user))
                {
                        ConfigTag* mtag = memb->user->oper->type_block;
                        if (mtag->getString("immunity") == "yes")
                        {
                                src->WriteNumeric(484, "%s %s :You are not permitted to kick Super IRCOPs",
                                        src->nick.c_str(), memb->chan->name.c_str());
                                return MOD_RES_DENY;
                        }
                }
                return MOD_RES_PASSTHRU;
        }
};
MODULE_INIT(ModuleServProtect)
