#!/usr/bin/perl -wT
# -*- Mode: perl; indent-tabs-mode: nil -*-
#
# The contents of this file are subject to the Mozilla Public
# License Version 1.1 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
#
# The Original Code is the Bugzilla Bug Tracking System.
#
# Contributor(s): Marc Schumann <wurblzap@gmail.com>

use strict;
use lib ".";

require "CGI.pl";
require "globals.pl";

use vars qw( $vars );

use Bugzilla;
use Bugzilla::User;
use Bugzilla::Config;
use Bugzilla::Constants;
use Bugzilla::Auth;
use Bugzilla::Util;

Bugzilla->login(LOGIN_REQUIRED);

my $cgi       = Bugzilla->cgi;
my $template  = Bugzilla->template;
my $dbh       = Bugzilla->dbh;
my $user      = Bugzilla->user;
my $userid    = $user->id;
my $editusers = $user->in_group('editusers');

# Reject access if there is no sense in continuing.
$editusers
    || Bugzilla->user->can_bless()
    || ThrowUserError("auth_failure", {group  => "editusers",
                                       reason => "cant_bless",
                                       action => "edit",
                                       object => "users"});

print Bugzilla->cgi->header();

# Common CGI params
my $action       = $cgi->param('action') || 'search';
my $login        = $cgi->param('login');
my $password     = $cgi->param('password');
my $groupid      = $cgi->param('groupid');
my $otherUser    = new Bugzilla::User($cgi->param('userid'));
my $realname     = trim($cgi->param('name')         || '');
my $disabledtext = trim($cgi->param('disabledtext') || '');

# Directly from common CGI params derived values
my $otherUserID = $otherUser->id();

# Prefill template vars with data used in all or nearly all templates
$vars->{'editusers'} = $editusers;
mirrorListSelectionValues();

###########################################################################
if ($action eq 'search') {
    # Allow to restrict the search to any group the user is allowed to bless.
    $vars->{'restrictablegroups'} = groupsUserMayBless($user, 'id', 'name');
    $template->process('admin/users/search.html.tmpl', $vars)
       || ThrowTemplateError($template->error());

###########################################################################
} elsif ($action eq 'list') {
    my $matchstr      = $cgi->param('matchstr');
    my $matchtype     = $cgi->param('matchtype');
    my $grouprestrict = $cgi->param('grouprestrict') || '0';
    my $query = 'SELECT DISTINCT userid, login_name, realname, disabledtext ' .
                'FROM profiles';
    my @bindValues;
    my $nextCondition;

    if (Param('usevisibilitygroups')) {
        # Show only users in visible groups.
        my $visibleGroups = visibleGroupsAsString();
        $query .= qq{, user_group_map AS ugm
                     WHERE ugm.user_id = profiles.userid
                       AND ugm.isbless = 0
                       AND ugm.group_id IN ($visibleGroups)
                    };
        $nextCondition = 'AND';
    } else {
        if ($grouprestrict eq '1') {
            $query .= ', user_group_map AS ugm';
        }
        $nextCondition = 'WHERE';
    }

    # Selection by user name.
    if (defined($matchtype)) {
        $query .= " $nextCondition profiles.login_name ";
        if ($matchtype eq 'regexp') {
            $query .= $dbh->sql_regexp . ' ?';
            $matchstr = '.' unless $matchstr;
        } elsif ($matchtype eq 'notregexp') {
            $query .= $dbh->sql_not_regexp . ' ?';
            $matchstr = '.' unless $matchstr;
        } else { # substr or unknown
            $query .= 'like ?';
            $matchstr = "%$matchstr%";
        }
        $nextCondition = 'AND';
        # We can trick_taint because we use the value in a SELECT only, using
        # a placeholder.
        trick_taint($matchstr);
        push(@bindValues, $matchstr);
    }

    # Selection by group.
    if ($grouprestrict eq '1') {
        $query .= " $nextCondition profiles.userid = ugm.user_id " .
                  'AND ugm.group_id = ?';
        # We can trick_taint because we use the value in a SELECT only, using
        # a placeholder.
        trick_taint($groupid);
        push(@bindValues, $groupid);
    }
    $query .= ' ORDER BY profiles.login_name';

    $vars->{'users'} = $dbh->selectall_arrayref($query,
                                                {'Slice' => {}},
                                                @bindValues);
    $template->process('admin/users/list.html.tmpl', $vars)
       || ThrowTemplateError($template->error());

###########################################################################
} elsif ($action eq 'add') {
    $editusers || ThrowUserError("auth_failure", {group  => "editusers",
                                                  action => "add",
                                                  object => "users"});

    $template->process('admin/users/create.html.tmpl', $vars)
       || ThrowTemplateError($template->error());

###########################################################################
} elsif ($action eq 'new') {
    $editusers || ThrowUserError("auth_failure", {group  => "editusers",
                                                  action => "add",
                                                  object => "users"});

    # Lock tables during the check+creation session.
    $dbh->bz_lock_tables('profiles WRITE',
                         'profiles_activity WRITE',
                         'namedqueries READ',
                         'whine_queries READ',
                         'tokens READ');

    # Validity checks
    $login || ThrowUserError('user_login_required');
    CheckEmailSyntax($login);
    is_available_username($login) || ThrowUserError('account_exists',
                                                    {'email' => $login});
    ValidatePassword($password);

    # Login and password are validated now, and realname and disabledtext
    # are allowed to contain anything
    trick_taint($login);
    trick_taint($realname);
    trick_taint($password);
    trick_taint($disabledtext);

    insert_new_user($login, $realname, $password, $disabledtext);
    my $userid = $dbh->bz_last_key('profiles', 'userid');
    $dbh->bz_unlock_tables();
    userDataToVars($userid);

    $vars->{'message'} = 'account_created';
    $template->process('admin/users/edit.html.tmpl', $vars)
       || ThrowTemplateError($template->error());

###########################################################################
} elsif ($action eq 'edit') {
    $otherUser 
        || ThrowCodeError('invalid_user_id', {'userid' => $cgi->param('userid')});

    canSeeUser($otherUserID)
        || ThrowUserError('auth_failure', {reason => "not_visible",
                                           action => "modify",
                                           object => "user"});

    userDataToVars($otherUserID);

    $template->process('admin/users/edit.html.tmpl', $vars)
       || ThrowTemplateError($template->error());

###########################################################################
} elsif ($action eq 'update') {
    $otherUser
        || ThrowCodeError('invalid_user_id', {'userid' => $cgi->param('userid')});
    my $logoutNeeded = 0;
    my @changedFields;

    # Lock tables during the check+update session.
    $dbh->bz_lock_tables('profiles WRITE',
                         'profiles_activity WRITE',
                         'fielddefs READ',
                         'namedqueries READ',
                         'whine_queries READ',
                         'tokens WRITE',
                         'logincookies WRITE',
                         'groups READ',
                         'user_group_map WRITE',
                         'group_group_map READ');
 
    canSeeUser($otherUserID)
        || ThrowUserError('auth_failure', {reason => "not_visible",
                                           action => "modify",
                                           object => "user"});

    # Cleanups
    my $loginold        = $cgi->param('loginold')        || '';
    my $realnameold     = $cgi->param('nameold')         || '';
    my $password        = $cgi->param('password')        || '';
    my $disabledtextold = $cgi->param('disabledtextold') || '';

    # Update profiles table entry; silently skip doing this if the user
    # is not authorized.
    if ($editusers) {
        my @values;

        if ($login ne $loginold) {
            # Validate, then trick_taint.
            $login || ThrowUserError('user_login_required');
            CheckEmailSyntax($login);
            is_available_username($login) || ThrowUserError('account_exists',
                                                            {'email' => $login});
            trick_taint($login);
            push(@changedFields, 'login_name');
            push(@values, $login);
            $logoutNeeded = 1;

            # Since we change the login, silently delete any tokens.
            $dbh->do('DELETE FROM tokens WHERE userid = ?', {}, $otherUserID);
        }
        if ($realname ne $realnameold) {
            # The real name may be anything; we use a placeholder for our
            # INSERT, and we rely on displaying code to FILTER html.
            trick_taint($realname);
            push(@changedFields, 'realname');
            push(@values, $realname);
        }
        if ($password) {
            # Validate, then trick_taint.
            ValidatePassword($password) if $password;
            trick_taint($password);
            push(@changedFields, 'cryptpassword');
            push(@values, bz_crypt($password));
            $logoutNeeded = 1;
        }
        if ($disabledtext ne $disabledtextold) {
            # The disable text may be anything; we use a placeholder for our
            # INSERT, and we rely on displaying code to FILTER html.
            trick_taint($disabledtext);
            push(@changedFields, 'disabledtext');
            push(@values, $disabledtext);
            $logoutNeeded = 1;
        }
        if (@changedFields) {
            push (@values, $otherUserID);
            $logoutNeeded && Bugzilla->logout_user_by_id($otherUserID);
            $dbh->do('UPDATE profiles SET ' .
                     join(' = ?,', @changedFields).' = ? ' .
                     'WHERE userid = ?',
                     undef, @values);
            # XXX: should create profiles_activity entries.
        }
    }

    # Update group settings.
    my $sth_add_mapping = $dbh->prepare(
        qq{INSERT INTO user_group_map (
                  user_id, group_id, isbless, grant_type
                 ) VALUES (
                  ?, ?, ?, ?
                 )
          });
    my $sth_remove_mapping = $dbh->prepare(
        qq{DELETE FROM user_group_map
            WHERE user_id = ?
              AND group_id = ?
              AND isbless = ?
              AND grant_type = ?
          });

    my @groupsAddedTo;
    my @groupsRemovedFrom;
    my @groupsGrantedRightsToBless;
    my @groupsDeniedRightsToBless;

    # Regard only groups the user is allowed to bless and skip all others
    # silently.
    # XXX: checking for existence of each user_group_map entry
    #      would allow to display a friendlier error message on page reloads.
    foreach (@{groupsUserMayBless($user, 'id', 'name')}) {
        my $id = $$_{'id'};
        my $name = $$_{'name'};

        # Change memberships.
        my $oldgroupid = $cgi->param("oldgroup_$id") || '0';
        my $groupid    = $cgi->param("group_$id")    || '0';
        if ($groupid ne $oldgroupid) {
            if ($groupid eq '0') {
                $sth_remove_mapping->execute(
                    $otherUserID, $id, 0, GRANT_DIRECT);
                push(@groupsRemovedFrom, $name);
            } else {
                $sth_add_mapping->execute(
                    $otherUserID, $id, 0, GRANT_DIRECT);
                push(@groupsAddedTo, $name);
            }
        }

        # Only members of the editusers group may change bless grants.
        # Skip silently if this is not the case.
        if ($editusers) {
            my $oldgroupid = $cgi->param("oldbless_$id") || '0';
            my $groupid    = $cgi->param("bless_$id")    || '0';
            if ($groupid ne $oldgroupid) {
                if ($groupid eq '0') {
                    $sth_remove_mapping->execute(
                        $otherUserID, $id, 1, GRANT_DIRECT);
                    push(@groupsDeniedRightsToBless, $name);
                } else {
                    $sth_add_mapping->execute(
                        $otherUserID, $id, 1, GRANT_DIRECT);
                    push(@groupsGrantedRightsToBless, $name);
                }
            }
        }
    }
    if (@groupsAddedTo || @groupsRemovedFrom) {
        $dbh->do(qq{INSERT INTO profiles_activity (
                           userid, who,
                           profiles_when, fieldid,
                           oldvalue, newvalue
                          ) VALUES (
                           ?, ?, now(), ?, ?, ?
                          )
                   },
                 undef,
                 ($otherUserID, $userid,
                  GetFieldID('bug_group'),
                  join(', ', @groupsRemovedFrom), join(', ', @groupsAddedTo)));
        $dbh->do('UPDATE profiles SET refreshed_when=? WHERE userid = ?',
                 undef, ('1900-01-01 00:00:00', $otherUserID));
    }
    # XXX: should create profiles_activity entries for blesser changes.

    $dbh->bz_unlock_tables();

    # XXX: userDataToVars may be off when editing ourselves.
    userDataToVars($otherUserID);

    $vars->{'message'} = 'account_updated';
    $vars->{'loginold'} = $loginold;
    $vars->{'changed_fields'} = \@changedFields;
    $vars->{'groups_added_to'} = \@groupsAddedTo;
    $vars->{'groups_removed_from'} = \@groupsRemovedFrom;
    $vars->{'groups_granted_rights_to_bless'} = \@groupsGrantedRightsToBless;
    $vars->{'groups_denied_rights_to_bless'} = \@groupsDeniedRightsToBless;
    $template->process('admin/users/edit.html.tmpl', $vars)
       || ThrowTemplateError($template->error());

###########################################################################
} elsif ($action eq 'del') {
    $otherUser
        || ThrowCodeError('invalid_user_id', {'userid' => $cgi->param('userid')});

    Param('allowuserdeletion') || ThrowUserError('users_deletion_disabled');
    $editusers || ThrowUserError('auth_failure', {group  => "editusers",
                                                  action => "delete",
                                                  object => "users"});
    canSeeUser($otherUserID) || ThrowUserError('auth_failure',
                                               {reason => "not_visible",
                                                action => "delete",
                                                object => "user"});

    $vars->{'otheruser'}      = $otherUser;
    $vars->{'editcomponents'} = UserInGroup('editcomponents');

    # If the user is initial owner or initial QA contact of a component,
    # then no deletion is possible.
    $vars->{'product_responsibilities'} = productResponsibilities($otherUserID);

    # Find other cross references.
    $vars->{'bugs'} = $dbh->selectrow_array(
        qq{SELECT COUNT(*)
           FROM bugs
           WHERE assigned_to = ? OR
                 qa_contact = ? OR
                 reporter = ?
          },
        undef, ($otherUserID, $otherUserID, $otherUserID));
    $vars->{'cc'} = $dbh->selectrow_array(
        'SELECT COUNT(*) FROM cc WHERE who = ?',
        undef, $otherUserID);
    $vars->{'bugs_activity'} = $dbh->selectrow_array(
        'SELECT COUNT(*) FROM bugs_activity WHERE who = ?',
        undef, $otherUserID);
    $vars->{'flags'}{'requestee'} = $dbh->selectrow_array(
        'SELECT COUNT(*) FROM flags WHERE requestee_id = ?',
        undef, $otherUserID);
    $vars->{'flags'}{'setter'} = $dbh->selectrow_array(
        'SELECT COUNT(*) FROM flags WHERE setter_id = ?',
        undef, $otherUserID);
    $vars->{'longdescs'} = $dbh->selectrow_array(
        'SELECT COUNT(*) FROM longdescs WHERE who = ?',
        undef, $otherUserID);
    $vars->{'namedqueries'} = $dbh->selectrow_array(
        'SELECT COUNT(*) FROM namedqueries WHERE userid = ?',
        undef, $otherUserID);
    $vars->{'profiles_activity'} = $dbh->selectrow_array(
        'SELECT COUNT(*) FROM profiles_activity WHERE who = ? AND userid != ?',
        undef, ($otherUserID, $otherUserID));
    $vars->{'series'} = $dbh->selectrow_array(
        'SELECT COUNT(*) FROM series WHERE creator = ?',
        undef, $otherUserID);
    $vars->{'votes'} = $dbh->selectrow_array(
        'SELECT COUNT(*) FROM votes WHERE who = ?',
        undef, $otherUserID);
    $vars->{'watch'}{'watched'} = $dbh->selectrow_array(
        'SELECT COUNT(*) FROM watch WHERE watched = ?',
        undef, $otherUserID);
    $vars->{'watch'}{'watcher'} = $dbh->selectrow_array(
        'SELECT COUNT(*) FROM watch WHERE watcher = ?',
        undef, $otherUserID);
    $vars->{'whine_events'} = $dbh->selectrow_array(
        'SELECT COUNT(*) FROM whine_events WHERE owner_userid = ?',
        undef, $otherUserID);
    $vars->{'whine_schedules'} = $dbh->selectrow_array(
        qq{SELECT COUNT(distinct eventid)
           FROM whine_schedules
           WHERE mailto = ?
           AND mailto_type = ?
          },
        undef, ($otherUserID, MAILTO_USER));

    $template->process('admin/users/confirm-delete.html.tmpl', $vars)
       || ThrowTemplateError($template->error());

###########################################################################
} elsif ($action eq 'delete') {
    $otherUser
        || ThrowCodeError('invalid_user_id', {'userid' => $cgi->param('userid')});
    my $otherUserLogin = $otherUser->login();

    # Lock tables during the check+removal session.
    # XXX: if there was some change on these tables after the deletion
    #      confirmation checks, we may do something here we haven't warned
    #      about.
    $dbh->bz_lock_tables('products READ',
                         'components READ',
                         'logincookies WRITE',
                         'profiles WRITE',
                         'profiles_activity WRITE',
                         'groups READ',
                         'user_group_map WRITE',
                         'group_group_map READ',
                         'flags WRITE',
                         'cc WRITE',
                         'namedqueries WRITE',
                         'tokens WRITE',
                         'votes WRITE',
                         'watch WRITE',
                         'series WRITE',
                         'series_data WRITE',
                         'whine_schedules WRITE',
                         'whine_queries WRITE',
                         'whine_events WRITE');

    Param('allowuserdeletion')
        || ThrowUserError('users_deletion_disabled');
    $editusers || ThrowUserError('auth_failure',
                                 {group  => "editusers",
                                  action => "delete",
                                  object => "users"});
    canSeeUser($otherUserID) || ThrowUserError('auth_failure',
                                               {reason => "not_visible",
                                                action => "delete",
                                                object => "user"});
    productResponsibilities($otherUserID)
        && ThrowUserError('user_has_responsibility');

    Bugzilla->logout_user_by_id($otherUserID);

    # Reference removals.
    $dbh->do('UPDATE flags set requestee_id = NULL WHERE requestee_id = ?',
             undef, $otherUserID);

    # Simple deletions in referred tables.
    $dbh->do('DELETE FROM cc WHERE who = ?', undef, $otherUserID);
    $dbh->do('DELETE FROM logincookies WHERE userid = ?', undef, $otherUserID);
    $dbh->do('DELETE FROM namedqueries WHERE userid = ?', undef, $otherUserID);
    $dbh->do('DELETE FROM profiles_activity WHERE userid = ? OR who = ?', undef,
             ($otherUserID, $otherUserID));
    $dbh->do('DELETE FROM tokens WHERE userid = ?', undef, $otherUserID);
    $dbh->do('DELETE FROM user_group_map WHERE user_id = ?', undef,
             $otherUserID);
    $dbh->do('DELETE FROM votes WHERE who = ?', undef, $otherUserID);
    $dbh->do('DELETE FROM watch WHERE watcher = ? OR watched = ?', undef,
             ($otherUserID, $otherUserID));

    # More complex deletions in referred tables.
    my $id;

    # 1) Series
    my $sth_seriesid = $dbh->prepare(
           'SELECT series_id FROM series WHERE creator = ?');
    my $sth_deleteSeries = $dbh->prepare(
           'DELETE FROM series WHERE series_id = ?');
    my $sth_deleteSeriesData = $dbh->prepare(
           'DELETE FROM series_data WHERE series_id = ?');

    $sth_seriesid->execute($otherUserID);
    while ($id = $sth_seriesid->fetchrow_array()) {
        $sth_deleteSeriesData->execute($id);
        $sth_deleteSeries->execute($id);
    }

    # 2) Whines
    my $sth_whineidFromSchedules = $dbh->prepare(
           qq{SELECT eventid FROM whine_schedules
              WHERE mailto = ? AND mailto_type = ?});
    my $sth_whineidFromEvents = $dbh->prepare(
           'SELECT id FROM whine_events WHERE owner_userid = ?');
    my $sth_deleteWhineEvent = $dbh->prepare(
           'DELETE FROM whine_events WHERE id = ?');
    my $sth_deleteWhineQuery = $dbh->prepare(
           'DELETE FROM whine_queries WHERE eventid = ?');
    my $sth_deleteWhineSchedule = $dbh->prepare(
           'DELETE FROM whine_schedules WHERE eventid = ?');

    $sth_whineidFromSchedules->execute($otherUserID, MAILTO_USER);
    while ($id = $sth_whineidFromSchedules->fetchrow_array()) {
        $sth_deleteWhineQuery->execute($id);
        $sth_deleteWhineSchedule->execute($id);
        $sth_deleteWhineEvent->execute($id);
    }

    $sth_whineidFromEvents->execute($otherUserID);
    while ($id = $sth_whineidFromEvents->fetchrow_array()) {
        $sth_deleteWhineQuery->execute($id);
        $sth_deleteWhineSchedule->execute($id);
        $sth_deleteWhineEvent->execute($id);
    }

    # Finally, remove the user account itself.
    $dbh->do('DELETE FROM profiles WHERE userid = ?', undef, $otherUserID);

    $dbh->bz_unlock_tables();

    $vars->{'message'} = 'account_deleted';
    $vars->{'otheruser'}{'login'} = $otherUserLogin;
    $vars->{'restrictablegroups'} = groupsUserMayBless($user, 'id', 'name');
    $template->process('admin/users/search.html.tmpl', $vars)
       || ThrowTemplateError($template->error());

###########################################################################
} else {
    $vars->{'action'} = $action;
    ThrowCodeError('action_unrecognized', $vars);
}

exit;

###########################################################################
# Helpers
###########################################################################

# Copy incoming list selection values from CGI params to template variables.
sub mirrorListSelectionValues {
    if (defined($cgi->param('matchtype'))) {
        foreach ('matchstr', 'matchtype', 'grouprestrict', 'groupid') {
            $vars->{'listselectionvalues'}{$_} = $cgi->param($_);
        }
    }
}

# Give a list of IDs of groups the user can see.
sub visibleGroupsAsString {
    return join(', ', -1, @{$user->visible_groups_direct()});
}

# Give a list of IDs of groups the user may bless.
sub groupsUserMayBless {
    my $user = shift;
    my $fieldList = join(', ', @_);
    my $query;
    my $connector;
    my @bindValues;

    $user->derive_groups(1);

    if ($editusers) {
        $query = "SELECT DISTINCT $fieldList FROM groups";
        $connector = 'WHERE';
    } else {
        $query = qq{SELECT DISTINCT $fieldList
                    FROM groups, user_group_map AS ugm
                    LEFT JOIN group_group_map AS ggm
                           ON ggm.member_id = ugm.group_id
                          AND ggm.grant_type = ?
                    WHERE user_id = ?
                      AND ((id = group_id AND isbless = 1) OR
                           (id = grantor_id))
                   };
        @bindValues = (GROUP_BLESS, $userid);
        $connector = 'AND';
    }

    # If visibilitygroups are used, restrict the set of groups.
    if (Param('usevisibilitygroups')) {
        my $visibleGroups = visibleGroupsAsString();
        $query .= " $connector id in ($visibleGroups)";
    }

    $query .= ' ORDER BY name';

    return $dbh->selectall_arrayref($query, {'Slice' => {}}, @bindValues);
}

# Determine whether the user can see a user. (Checks for existence, too.)
sub canSeeUser {
    my $otherUserID = shift;
    my $query;

    if (Param('usevisibilitygroups')) {
        my $visibleGroups = visibleGroupsAsString();
        $query = qq{SELECT COUNT(DISTINCT userid)
                    FROM profiles, user_group_map
                    WHERE userid = ?
                    AND user_id = userid
                    AND isbless = 0
                    AND group_id IN ($visibleGroups)
                   };
    } else {
        $query = qq{SELECT COUNT(userid)
                    FROM profiles
                    WHERE userid = ?
                   };
    }
    return $dbh->selectrow_array($query, undef, $otherUserID);
}

# Retrieve product responsibilities, usable for both display and verification.
sub productResponsibilities {
    my $userid = shift;
    my $h = $dbh->selectall_arrayref(
           qq{SELECT products.name AS productname,
                     components.name AS componentname,
                     initialowner,
                     initialqacontact
              FROM products, components
              WHERE products.id = components.product_id
                AND ? IN (initialowner, initialqacontact)
             },
           {'Slice' => {}}, $userid);

    if (@$h) {
        return $h;
    } else {
        return undef;
    }
}

# Retrieve user data for the user editing form. User creation and user
# editing code rely on this to call derive_groups().
sub userDataToVars {
    my $userid = shift;
    my $user = new Bugzilla::User($userid);
    my $query;
    my $dbh = Bugzilla->dbh;

    $user->derive_groups();

    $vars->{'otheruser'} = $user;
    $vars->{'groups'} = groupsUserMayBless($user, 'id', 'name', 'description');
    $vars->{'disabledtext'} = $dbh->selectrow_array(
        'SELECT disabledtext FROM profiles WHERE userid = ?', undef, $userid);

    $vars->{'permissions'} = $dbh->selectall_hashref(
        qq{SELECT id,
                  COUNT(directmember.group_id) AS directmember,
                  COUNT(regexpmember.group_id) AS regexpmember,
                  COUNT(derivedmember.group_id) AS derivedmember,
                  COUNT(directbless.group_id) AS directbless
           FROM groups
           LEFT JOIN user_group_map AS directmember
                  ON directmember.group_id = id
                 AND directmember.user_id = ?
                 AND directmember.isbless = 0
                 AND directmember.grant_type = ?
           LEFT JOIN user_group_map AS regexpmember
                  ON regexpmember.group_id = id
                 AND regexpmember.user_id = ?
                 AND regexpmember.isbless = 0
                 AND regexpmember.grant_type = ?
           LEFT JOIN user_group_map AS derivedmember
                  ON derivedmember.group_id = id
                 AND derivedmember.user_id = ?
                 AND derivedmember.isbless = 0
                 AND derivedmember.grant_type = ?
           LEFT JOIN user_group_map AS directbless
                  ON directbless.group_id = id
                 AND directbless.user_id = ?
                 AND directbless.isbless = 1
                 AND directbless.grant_type = ?
          } . $dbh->sql_group_by('id'),
        'id', undef,
        ($userid, GRANT_DIRECT,
         $userid, GRANT_REGEXP,
         $userid, GRANT_DERIVED,
         $userid, GRANT_DIRECT));

    # Find indirect bless permission.
    $query = qq{SELECT groups.id
                FROM groups, user_group_map AS ugm, group_group_map AS ggm
                WHERE ugm.user_id = ?
                  AND groups.id = ggm.grantor_id
                  AND ggm.member_id = ugm.group_id
                  AND ugm.isbless = 0
                  AND ggm.grant_type = ?
               } . $dbh->sql_group_by('id');
    foreach (@{$dbh->selectall_arrayref($query, undef, ($userid, GROUP_BLESS))}) {
        # Merge indirect bless permissions into permission variable.
        $vars->{'permissions'}{${$_}[0]}{'indirectbless'} = 1;
    }
}
