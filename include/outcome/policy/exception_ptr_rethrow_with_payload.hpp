/* Policies for result and outcome
(C) 2017 Niall Douglas <http://www.nedproductions.biz/> (59 commits)
File Created: Oct 2017


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License in the accompanying file
Licence.txt or at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


Distributed under the Boost Software License, Version 1.0.
(See accompanying file Licence.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef OUTCOME_POLICY_EXCEPTION_PTR_RETHROW_WITH_PAYLOAD_HPP
#define OUTCOME_POLICY_EXCEPTION_PTR_RETHROW_WITH_PAYLOAD_HPP

#include "../bad_access.hpp"
#include "detail/common.hpp"

OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

#ifdef STANDARDESE_IS_IN_THE_HOUSE
template <class R, class S, class P, class N> class outcome;
#endif

namespace policy
{
  /*! Policy interpreting S as a type implementing the `std::exception_ptr` contract
  and any wide attempt to access the successful state calls an
  ADL discovered free function `throw_exception_ptr_with_payload()`.

  Can be used in `outcome` only.
  */
  template <class R, class S, class P> struct exception_ptr_rethrow_with_payload : detail::base
  {
    static_assert(std::is_base_of<std::exception_ptr, S>::value, "error_type must be a base of a std::exception_ptr to be used with this policy");
    /*! Performs a wide check of state, used in the value() functions
    if has an error it throws a `std::system_error(error())`, else it throws `bad_outcome_access`.
    */
    template <class Impl> static constexpr void wide_value_check(Impl *self)
    {
      if((self->_state._status & OUTCOME_V2_NAMESPACE::detail::status_have_value) == 0)
      {
        if((self->_state._status & OUTCOME_V2_NAMESPACE::detail::status_have_payload) != 0)
        {
          auto *_self = static_cast<const outcome<R, S, P, exception_ptr_rethrow_with_payload> *>(self);
          throw_exception_ptr_with_payload(_self);
        }
        if((self->_state._status & OUTCOME_V2_NAMESPACE::detail::status_have_error) != 0)
        {
          std::rethrow_exception(self->_error);
        }
        OUTCOME_THROW_EXCEPTION(bad_outcome_access("no value"));
      }
    }
    /*! Performs a wide check of state, used in the error() functions
    \effects If outcome does not have an error, it throws `bad_outcome_access`.
    */
    template <class Impl> static constexpr void wide_error_check(Impl *self)
    {
      if((self->_state._status & OUTCOME_V2_NAMESPACE::detail::status_have_error) == 0)
      {
        OUTCOME_THROW_EXCEPTION(bad_outcome_access("no error"));
      }
    }
    /*! Performs a wide check of state, used in the payload() functions
    \effects If outcome does not have a payload, it throws `bad_outcome_access`.
    */
    template <class Impl> static constexpr void wide_payload_check(Impl *self)
    {
      if((self->_state._status & OUTCOME_V2_NAMESPACE::detail::status_have_payload) == 0)
      {
        OUTCOME_THROW_EXCEPTION(bad_outcome_access("no payload"));
      }
    }
    /*! Performs a wide check of state, used in the exception() functions
    \effects If outcome does not have an exception, it throws `bad_outcome_access`.
    */
    template <class Impl> static constexpr void wide_exception_check(Impl *self)
    {
      if((self->_state._status & OUTCOME_V2_NAMESPACE::detail::status_have_exception) == 0)
      {
        OUTCOME_THROW_EXCEPTION(bad_outcome_access("no exception"));
      }
    }
  };
}  // namespace policy

OUTCOME_V2_NAMESPACE_END

#endif
