// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015-2016 Oslo and Akershus University College of Applied Sciences
// and Alfred Bratterud
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ROUTES_SQUIRREL_ROUTER_HPP
#define ROUTES_SQUIRREL_ROUTER_HPP
#include <router.hpp>
#include <models/squirrel.hpp>
#include <bucket.hpp>
#include <json.hpp>



namespace acorn {

class Squirrel_router : public server::Router {

  using SquirrelBucket = bucket::Bucket<Squirrel>;

public:

  Squirrel_router(std::shared_ptr<SquirrelBucket> squirrels)
  {
    // GET /
    on_get("/",
    [squirrels] (auto, auto res)
    {
      printf("[Squirrels@GET:/] Responding with content inside SquirrelBucket\n");
      using namespace rapidjson;
      StringBuffer sb;
      Writer<StringBuffer> writer(sb);
      squirrels->serialize(writer);
      res->send_json(sb.GetString());
    });

    // POST /
    on_post("/",
    [squirrels] (server::Request_ptr req, auto res)
    {
      using namespace json;
      auto json = req->get_attribute<Json_doc>();
      if(!json) {
        res->error({http::Internal_Server_Error, "Server Error", "Server needs to be sprinkled with Parsley"});
      }
      else {
        auto& doc = json->doc();
        try {
          // create an empty model
          acorn::Squirrel s;
          // deserialize it
          s.deserialize(doc);
          // add to bucket
          auto id = squirrels->capture(s);
          assert(id == s.key);
          printf("[Squirrels@POST:/] Squirrel captured: %s\n", s.get_name().c_str());
          // setup the response
          // location to the newly created resource
          using namespace std;
          res->add_header(http::header_fields::Response::Location, req->uri().path()); // return back end loc i guess?
          // status code 201 Created
          res->set_status_code(http::Created);
          // send the created entity as response
          res->send_json(s.json());
        }
        catch(Assert_error e) {
          printf("[Squirrels@POST:/] Assert_error: %s\n", e.what());
          res->error({"Parsing Error", "Could not parse data."});
        }
        catch(bucket::ConstraintException e) {
          printf("[Squirrels@POST:/] ConstraintException: %s\n", e.what());
          res->error({"Constraint Exception", e.what()});
        }
        catch(bucket::BucketException e) {
          printf("[Squirrels@POST:/] BucketException: %s\n", e.what());
          res->error({"Bucket Exception", e.what()});
        }
      }
    });

  }
};

}

#endif