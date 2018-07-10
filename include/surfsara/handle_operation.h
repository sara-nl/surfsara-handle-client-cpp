/*
MIT License

Copyright (c) 2018 SURFsara

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once
#include <surfsara/ast.h>

namespace surfsara
{
  namespace handle
  {
    using Node = ::surfsara::ast::Node;
    
    class Operation
    {
    public:
      inline bool isUpdate() const;
      inline bool isDelete() const;
      inline std::string getPath() const;
      inline Node getNode() const;
      static Node operations2list(const std::vector<Operation> & operations);
    private:
      friend Operation Update(const std::string & path, const Node & node);
      friend Operation Delete(const std::string & path);
      Operation(const std::string & path);
      Operation(const std::string & path, const Node & node);
      bool _isDelete;
      std::string url;
      Node node;
    };

    Operation Update(const std::string & path, const Node & node);
    Operation Delete(const std::string & path);

  }
}

////////////////////////////////////////////////////////////////////////////////
//
// implementation
//
////////////////////////////////////////////////////////////////////////////////
inline surfsara::handle::Operation::Operation(const std::string & _url)
  : _isDelete(true), url(_url)
{
}


inline surfsara::handle::Operation::Operation(const std::string & _url,
                                              const Node & _node)
  : _isDelete(false), url(_url), node(_node)
{
}

inline surfsara::ast::Node surfsara::handle::Operation::getNode() const
{
  return node;
}

inline bool surfsara::handle::Operation::isUpdate() const
{
  return !_isDelete;
}

inline bool surfsara::handle::Operation::isDelete() const
{
  return _isDelete;
}

#include <iostream>
#include <surfsara/json_format.h>
surfsara::ast::Node
surfsara::handle::Operation::operations2list(const std::vector<Operation> & operations)
{
  using namespace ::surfsara::ast;
  Node ret(Array({}));
  for(auto op : operations)
  {
    if(op.isUpdate())
    {
      Node n(op.getNode());
      //if(n.isA<surfsara::ast::Object>())
      //{
      //  n.as<surfsara::ast::Object>().set("index", Integer(1));
      //}
      //ret.as<surfsara::ast::Array>().pushBack(op.getNode());
      //n.set("index", op.getPath());
      //op.set("index", op.get const String & k, Node && node);
      //nodes.push_back(n);
    }
  }
  std::cout << formatJson(ret) << std::endl;
  return ret;
}

surfsara::handle::Operation surfsara::handle::Update(const std::string & url,
                                                     const Node & node)
{
  return Operation(url, node);
}

surfsara::handle::Operation surfsara::handle::Delete(const std::string & url)
{
  return Operation(url);
}


