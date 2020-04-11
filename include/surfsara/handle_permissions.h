#pragma once
#include <string>
#include <vector>
#include <set>

namespace surfsara
{
  namespace handle
  {
    class Permissions
    {
    public:
      Permissions(const std::vector<std::string> & users,
                  const std::vector<std::string> & groups);
      /**
       * Check if permission is granted for any user or group.
       * Return true if permission is granted.
       */
      inline bool checkAny() const;

      /**
       * Return true if permission is granted for some users / groups.
       * The check must be completed with checkUser and checkGroup
       */
      inline bool checkSome() const;

      /**
       * Return true if permission is granted for user
       */
      inline bool checkOwner() const;

      /**
       * Return true if permission is granted for user
       */
      inline bool checkUser(const std::string & user) const;

      /**
       * Return true if permission is granted for group.
       */
      inline bool checkGroup(const std::string & group) const;

    private:
      bool all_users;
      bool all_groups;
      bool owners;
      std::set<std::string> users;
      std::set<std::string> groups;
    };
  }
}

inline surfsara::handle::Permissions::Permissions(const std::vector<std::string> & _users,
                                                  const std::vector<std::string> & _groups)
{
  all_users = false;
  all_groups = false;
  owners = false;
  for(const auto & u : _users)
  {
    if(u == "*")
    {
      all_users = true;
    }
    else if(u == "*owner*")
    {
      owners = true;
    }
    users.insert(u);
  }
  for(const auto & g : _groups)
  {
    if(g == "*")
    {
      all_groups = true;
    }
    else if(g == "*owner*")
    {
      owners = true;
    }
    groups.insert(g);
  }
}

inline bool surfsara::handle::Permissions::checkAny() const
{
  return all_users || all_groups;
}

inline bool surfsara::handle::Permissions::checkSome() const
{
  return !users.empty() || !groups.empty() || owners;
}

inline bool surfsara::handle::Permissions::checkOwner() const
{
  return owners;
}

inline bool surfsara::handle::Permissions::checkUser(const std::string & user) const
{
  return all_users || (users.find(user) != users.end());
}

inline bool surfsara::handle::Permissions::checkGroup(const std::string & group) const
{
  return all_groups || (groups.find(group) != groups.end());
}
