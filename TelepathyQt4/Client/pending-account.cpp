/*
 * This file is part of TelepathyQt4
 *
 * Copyright (C) 2008 Collabora Ltd. <http://www.collabora.co.uk/>
 * Copyright (C) 2008 Nokia Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <TelepathyQt4/Client/PendingAccount>

#include "TelepathyQt4/Client/_gen/pending-account.moc.hpp"

#include "TelepathyQt4/debug-internal.h"

#include <TelepathyQt4/Client/AccountManager>
#include <TelepathyQt4/Client/Account>

#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

/**
 * \addtogroup clientsideproxies Client-side proxies
 *
 * Proxy objects representing remote service objects accessed via D-Bus.
 *
 * In addition to providing direct access to methods, signals and properties
 * exported by the remote objects, some of these proxies offer features like
 * automatic inspection of remote object capabilities, property tracking,
 * backwards compatibility helpers for older services and other utilities.
 */

namespace Telepathy
{
namespace Client
{

struct PendingAccount::Private
{
    Private(AccountManager *manager) :
        manager(manager),
        account(0)
    {
    }

    AccountManager *manager;
    Account *account;
};

/**
 * \class PendingAccount
 * \ingroup clientaccount
 * \headerfile <TelepathyQt4/Client/pending-account.h> <TelepathyQt4/Client/PendingAccount>
 *
 * Class containing the parameters of and the reply to an asynchronous account
 * request. Instances of this class cannot be constructed directly; the only
 * way to get one is via AccountManager.
 */

/**
 * Construct a PendingAccount object.
 *
 * \param manager AccountManager to use.
 * \param connectionManager Name of the connection manager to create the account for.
 * \param protocol Name of the protocol to create the account for.
 * \param displayName Account display name.
 * \param parameters Account parameters.
 */
PendingAccount::PendingAccount(AccountManager *manager,
        const QString &connectionManager, const QString &protocol,
        const QString &displayName, const QVariantMap &parameters)
    : PendingOperation(manager),
      mPriv(new Private(manager))
{
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(
            manager->baseInterface()->CreateAccount(connectionManager,
                protocol, displayName, parameters), this);
    connect(watcher,
            SIGNAL(finished(QDBusPendingCallWatcher *)),
            SLOT(onCallFinished(QDBusPendingCallWatcher *)));
}

/**
 * Class destructor.
 */
PendingAccount::~PendingAccount()
{
    delete mPriv;
}

/**
 * Return the AccountManager object through which the request was made.
 *
 * \return Account Manager object.
 */
AccountManager *PendingAccount::manager() const
{
    return qobject_cast<AccountManager *>(parent());
}

/**
 * Returns the newly created Account object.
 *
 * \return Account object.
 */
Account *PendingAccount::account() const
{
    if (!isFinished()) {
        warning() <<
            "PendingAccount::account called before finished, returning 0";
        return 0;
    }

    return mPriv->account;
}

void PendingAccount::onCallFinished(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QDBusObjectPath> reply = *watcher;

    if (!reply.isError()) {
        debug() << "Got reply to AccountManager.CreateAccount";
        QDBusObjectPath objectPath = reply.value();
        debug() << "Creating account for objectPath: " << objectPath.path();
        mPriv->account = new Account(mPriv->manager,
                objectPath.path(), mPriv->manager);
        setFinished();
    } else {
        debug().nospace() <<
            "CreateAccount failed: " <<
            reply.error().name() << ": " << reply.error().message();
        setFinishedWithError(reply.error());
    }

    watcher->deleteLater();
}

} // Telepathy::Client
} // Telepathy