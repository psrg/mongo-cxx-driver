// @file init.h

/* Copyright 2013 10gen Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "mongo/base/status.h"
#include "mongo/client/export_macros.h"
#include "mongo/client/options.h"

// NOTE: These functions are only intended to be used when linking against the libmongoclient
// library. The below functions are not defined in servers like mongos or mongod, which have
// their own initialization strategy.

/**
 * @namespace mongo
 * @brief the main MongoDB namespace
 */
namespace mongo {

/**
 * @namespace mongo::client
 * @brief the MongoDB C++ driver namespace
 */
namespace client {

    /**
     *  Initializes the client driver, possibly with custom options. See the Options class for
     *  details on the various fields.
     *
     *  initialize() MUST be called EXACTLY once after entering 'main' and before using
     *  the driver. Do not call initialize() before entering 'main'
     *  (i.e. from a static initializer), as it relies on all static initialization
     *  having been completed.
     */
    MONGO_CLIENT_API Status MONGO_CLIENT_FUNC initialize(const Options& options = Options());

    /**
     *  Terminates the client driver. If the driver does not terminate within the currently
     *  configured grace period in the driver options, an 'ExceededTimeLimit' Status will be
     *  returned, in which case it is legal to retry 'shutdown'. Other non-OK status values do
     *  not admit retrying the operation. A permanent failure to terminate the driver should be
     *  logged, and it may be unsafe to exit the process by any mechanism which causes normal
     *  destruction of static objects.
     *
     *  Once the driver has been terminated, it cannot be initialized again.
     */
    MONGO_CLIENT_API Status MONGO_CLIENT_FUNC shutdown();

    /** An RAII helper to simplify driver setup and teardown. If more refined control over
     *  error handling of initialization and shutdown is required, use the explicit
     *  'initialize' and 'shutdown' functions above. Note that the restrictions on the
     *  invocation of 'initialize' are not obviated by using this class. Please see the methods
     *  below for more details.
     */
    class MONGO_CLIENT_API GlobalInstance {

        MONGO_DISALLOW_COPYING(GlobalInstance);

    public:
        /** Invokes 'mongo::client::initialize" with the provided Options, or the default
         *  Options if none are provided. If initialization is successful, destruction of the
         *  GlobalInstance will invoke 'shutdown' if such a call would not otherwise
         *  automatically be performed by the library via atexit (see
         *  Options::setCallShutdownAtExit). After constructing the GlobalInstance, you must
         *  check whether the initialization was successful via the 'initialized' method, or by
         *  checking the Status object returned by the 'status' member function. A failed
         *  GLobalInstance will not attempt to invoke shutdown.
         */
        explicit GlobalInstance(const Options& options = Options());

        /** Invokes 'mongo::client::shutdown' if the GlobalInstance succeeded in initializing
         *  the library and if the options used to successfully initialize the driver will not
         *  automatically result in a call to 'mongo::client::shutdown' during atexit
         *  processing.
         */
        ~GlobalInstance();

        /** Returns the Status generated by the internal call to 'client::initialize'. */
        const Status& status() const {
            return _status;
        }

        /** Returns 'true' if initialization succeeded, 'false' otherwise. */
        bool initialized() const {
            return status().isOK();
        }

        /** Raises a UserAssertion exception if the GlobalInstance failed to initialize the
         *  library.
         */
        void assertInitialized() const;

        /** Immediately calls 'shutdown' on the driver and returns the resulting Status. If the
         *  returned Status is OK, then the GlobalInstance will abandon the pending call to
         *  'shutdown' from its destructor. If the returned Status is not an OK status, then
         *  the pending call to 'shutdown' is not canceled. The call to
         *  GlobalInstance::shutdown may be retried if it returns a non-OK Status.
         */
        Status shutdown();

    private:
        bool _terminateNeeded;
        const Status _status;
    };

} // namespace client
} // namespace mongo
