//
// Copyright (c) 2014 CNRS
// Authors: Florent Lamiraux
//
// This file is part of hpp-core
// hpp-core is free software: you can redistribute it
// and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// hpp-core is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Lesser Public License for more details.  You should have
// received a copy of the GNU Lesser General Public License along with
// hpp-core  If not, see
// <http://www.gnu.org/licenses/>.

#include "extracted-path.hh"

#include <hpp/util/debug.hh>

#include <hpp/core/config-projector.hh>

namespace hpp {
  namespace core {
    // Constructor with constraints
    Path::Path (const interval_t& interval, size_type outputSize,
		size_type outputDerivativeSize,
		const ConstraintSetPtr_t& constraints) :
      paramRange_ (interval), timeRange_ (interval), outputSize_ (outputSize),
      outputDerivativeSize_ (outputDerivativeSize), constraints_ ()
    {
      if (constraints) {
	constraints_ = HPP_STATIC_PTR_CAST (ConstraintSet,
					    constraints->copy ());
      }
    }

    // Constructor without constraints
    Path::Path (const interval_t& interval, size_type outputSize,
		size_type outputDerivativeSize) :
      paramRange_ (interval), timeRange_ (interval), outputSize_ (outputSize),
      outputDerivativeSize_ (outputDerivativeSize), constraints_ ()
    {
    }

    // Copy constructor
    Path::Path (const Path& path) :
      paramRange_ (path.paramRange_), timeRange_ (path.timeRange_),
      outputSize_ (path.outputSize_),
      outputDerivativeSize_ (path.outputDerivativeSize_), constraints_ (),
      timeParam_ ()
    {
      if (path.constraints_) {
	constraints_ = HPP_STATIC_PTR_CAST (ConstraintSet,
					    path.constraints_->copy ());
      }
      if (path.timeParam_)
        timeParam_ = path.timeParam_->copy();
    }

    Path::Path (const Path& path, const ConstraintSetPtr_t& constraints) :
      paramRange_ (path.paramRange_), timeRange_ (path.timeRange_),
      outputSize_ (path.outputSize_),
      outputDerivativeSize_ (path.outputDerivativeSize_),
      constraints_ (constraints), timeParam_ ()
    {
      assert (!path.constraints_);
      if (path.timeParam_)
        timeParam_ = path.timeParam_->copy();
    }

    // Initialization after creation
    void Path::init (const PathWkPtr_t& self)
    {
      weak_ = self;
    }

    PathPtr_t Path::extract (const interval_t& subInterval) const
        throw (projection_error)
    {
      PathPtr_t res;
      if (timeParam_) {
        interval_t paramInterval (
            timeParam_->value(subInterval.first),
            timeParam_->value(subInterval.second));
        res = this->impl_extract (paramInterval);
        res->timeParameterization(timeParam_->copy(), subInterval);
      } else {
        res = this->impl_extract (subInterval);
      }
      return res;
    }

    PathPtr_t Path::impl_extract (const interval_t& paramInterval) const
        throw (projection_error)
    {
      if (subInterval == timeRange_)
	return this->copy ();
      return ExtractedPath::create (weak_.lock (), subInterval);
    }

    PathPtr_t Path::reverse () const
    {
      interval_t interval;
      interval.first = this->timeRange_.second;
      interval.second = this->timeRange_.first;
      return this->extract (interval);
    }

    void Path::checkPath () const
    {
      if (constraints()) {
        if (!constraints()->isSatisfied (initial())) {
          hppDout (error, *constraints());
          hppDout (error, initial().transpose ());
          throw projection_error ("Initial configuration of path does not satisfy "
              "the constraints");
        }
        if (constraints() && !constraints()->isSatisfied (end())) {
          hppDout (error, *constraints());
          hppDout (error, end().transpose ());
          throw projection_error ("End configuration of path does not satisfy "
              "the constraints");
        }
      }
    }

    std::ostream& Path::print (std::ostream& os) const
    {
      os << "time in [ " << timeRange().first << ", "
        << timeRange().second << " ]";
      if (timeParam_)
        os << ", param in [ " << paramRange().first << ", "
          << timeRange().second << " ]";
      os << std::endl;
      return os;
    }
  } //   namespace core
} // namespace hpp
