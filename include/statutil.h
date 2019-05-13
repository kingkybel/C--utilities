/*
 * File Name:   statutil.h
 * Description: statistic utility functions
 *
 * Copyright (C) 2019 Dieter J Kybelksties
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * @date: 2014-02-04
 * @author: Dieter J Kybelksties
 */

#ifndef NS_UTIL_STATUTIL_H_INCLUDED
#define NS_UTIL_STATUTIL_H_INCLUDED

#include <exception>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <utility>
#include <algorithm>
#include <cmath>
#include <boost/math/distributions/normal.hpp>
#include <boost/math/distributions/exponential.hpp>
#include <csvutil.h>
#include <anyutil.h>

namespace util
{

    /**
     * Error handling for event-range errors.
     */
    struct event_range_error : public std::logic_error
    {

        enum rangeType : byte
        {
            exponential_range, ///< Outside the range of the exponential function [0..oo)
            gaussian_range, ///< Outside the range of the gaussian function (-oo..oo)
            uniform_range ///< Outside the range of a uniform function [min..max]
        };

        event_range_error(VAR_FLOAT f, size_t rangeDescriptors)
        : std::logic_error("Floatrange-test for '" + asString(f) + "': the float description" +
                           " must be empty or exactly min and max values. Found "
                           + asString(rangeDescriptors))
        {
        }

        event_range_error(const std::string& str = "Invalid range")
        : std::logic_error(str)
        {
        }

        event_range_error(rangeType tp, VAR_FLOAT f1, VAR_FLOAT f2 = 0.0L, VAR_FLOAT f3 = 0.0L)
        : std::logic_error(
                           tp == exponential_range ?
                           "Range for exponential function is [0..oo) but found " + asString(f1) :
                           tp == uniform_range ?
                           "Range for exponential function is [" + asString(f1) + ".." + asString(f2) + "] but found " + asString(f3) :
                           "Invalid range.")
        {
        }
    };

    /**
     * Error handling for event errors.
     */
    struct event_error : public std::logic_error
    {

        event_error(const std::string& str)
        : std::logic_error(str)
        {
        }

        event_error(const std::type_info& t1, const std::type_info& t2)
        : std::logic_error(event_error("Cannot get interval of type "
                                       + asString(t1.name())
                                       + " from Event of type " + asString(t2.name())))
        {
        }
    };

    class EventList;

    /**
     * Error handling for event lists with mutually exclusive events.
     */
    struct eventlist_conflict_error : public std::logic_error
    {

        enum conflict_type : byte
        {
            evt, ///< Event list conflicts with itself
            cond, ///< condition list conflicts with itself
            evt_cond ///< evnet- and conditionlists conflict with each other
        };
        eventlist_conflict_error(conflict_type tp, const EventList& e1);
        eventlist_conflict_error(const EventList& e1, const EventList& e2);
    };

    /**
     * Error handling for general distribution errors like breach of probability requirements.
     */
    struct distribution_error : public std::logic_error
    {

        enum type : byte
        {
            empty_uniform, empty_normalise, empty_canonise
        };

        distribution_error(type tp)
        : std::logic_error(std::string(tp == empty_uniform ? "Make uniform" :
                                       tp == empty_normalise ? "Normalise" :
                                       "Canonise") +
                           ": cannot modify distribution as node-distribution "
                           "is empty and range is empty.")
        {
        }

        distribution_error(const std::string& err = "")
        : std::logic_error(err.empty() ? "Not a distribution!" : err)
        {
        }

        distribution_error(long double val)
        : std::logic_error("Probability value " + asString(val) + " is outside range [0.0..1.0]")
        {
        }
    };

    /**
     * Formalize one statistical Event.
     * Consists of the name of the event, a match-operation and a match-value.
     * <ul>
     * <li>x &le; 10</li>
     * <li>x in [-3.1415, 3.1415]</li>
     *  <li>y == "1st Jan 2014"</li>
     * </ul>
     */
    class Event
    {
    public:
        static Equals equals; ///< Default Equality operation
        static Less less; ///< Default Less operation
        static LessEqual lessEqual; ///< Default Less Equal operation
        static Greater greater; ///< Default Greater operation
        static GreaterEqual greaterEqual; ///< Default Greater Equal operation
        static IsElementOf isElementOf; ///< Default Element inclusion operation
        static PlaceHolderOp placeHolderOp; ///< Default Placeholder operation

        /**
         * Default construct empty Event.
         *
         * @param op Default-operation: Placeholder - returns false
         */
        Event(Operation* op = &placeHolderOp);

        /**
         * Construct boolean Event.
         *
         * @param name event name
         * @param b boolean value
         * @param op operation, default: Equality.
         */
        Event(const std::string& name, VAR_BOOL b, Operation* op = &equals);

        /**
         * Construct character Event.
         *
         * @param name event name
         * @param c character value
         * @param op operation, default: Equality.
         */
        Event(const std::string& name, VAR_CHAR c, Operation* op = &equals);

        /**
         * Construct signed integer Event.
         *
         * @param name event name
         * @param i integral value
         * @param op operation, default: Equality.
         */
        Event(const std::string& name, VAR_INT i, Operation* op = &equals);

        /**
         * Construct unsigned integer Event.
         *
         * @param name event name
         * @param u unsigned integral value
         * @param op operation, default: Equality.
         */
        Event(const std::string& name, VAR_UINT u, Operation* op = &equals);

        /**
         * Construct floating point Event.
         *
         * @param name event name
         * @param f floating point value
         * @param op operation, default: Equality.
         */
        Event(const std::string& name, VAR_FLOAT f, Operation* op = &equals);

        /**
         * Construct date Event.
         *
         * @param name event name
         * @param d date value
         * @param op operation, default: Equality.
         */
        Event(const std::string& name, VAR_DATE d, Operation* op = &equals);

        /**
         * Construct string Event.
         *
         * @param name event name
         * @param s string value
         * @param op operation, default: Equality.
         */
        Event(const std::string& name, VAR_STRING s, Operation* op = &equals);

        /**
         * Construct boolean interval Event.
         *
         * @param name event name
         * @param bi boolean interval value
         * @param op operation, default: Equality.
         */
        Event(const std::string& name, VAR_BOOL_INTERVAL bi, Operation* op = &isElementOf);

        /**
         * Construct character interval Event.
         *
         * @param name event name
         * @param ci character interval value
         * @param op operation, default: Containment.
         */
        Event(const std::string& name, VAR_CHAR_INTERVAL ci, Operation* op = &isElementOf);

        /**
         * Construct signed integer interval Event.
         *
         * @param name event name
         * @param ii integer interval value
         * @param op operation, default: Containment.
         */
        Event(const std::string& name, VAR_INT_INTERVAL ii, Operation* op = &isElementOf);

        /**
         * Construct unsigned integer interval Event.
         *
         * @param name event name
         * @param ui unsigned integer interval value
         * @param op operation, default: Containment.
         */
        Event(const std::string& name, VAR_UINT_INTERVAL ui, Operation* op = &isElementOf);

        /**
         * Construct floating point interval Event.
         *
         * @param name event name
         * @param fi floating point interval value
         * @param op operation, default: Containment..
         */
        Event(const std::string& name, VAR_FLOAT_INTERVAL fi, Operation* op = &isElementOf);

        /**
         * Construct date interval Event.
         *
         * @param name event name
         * @param di date interval value
         * @param op operation, default: Containment.
         */
        Event(const std::string& name, VAR_DATE_INTERVAL di, Operation* op = &isElementOf);

        /**
         * Construct character-string interval Event.
         *
         * @param name event name
         * @param si string interval value
         * @param op operation, default: Equality.
         */
        Event(const std::string& name, const char* si, Operation* op = &equals);

        /**
         * Construct Var-type Event.
         * @param name event name
         * @param a variant
         * @param dummyConfirm Needed to ensure only valid types are used.
         * @param op operation, default: Equality.
         */
        Event(const std::string& name, const Var& a, bool dummyConfirm, Operation* op = &equals);

        /**
         * Copy-construct Event.
         * @param rhs right-hand side
         */
        Event(const Event& rhs);

        /**
         * Assign Event.
         * @param rhs right-hand side
         */
        Event& operator=(const Event& rhs);

        /**
         * Change Event - parameters name/value/operation.
         *
         * @param name new name
         * @param value new value
         * @param op new operation
         * @return the modified *this
         */
        Event& operator()(const std::string& name = "",
                          const Var& value = Var(),
                          Operation* op = &equals);


        /**
         * Explicitly create an empty (placeholder) Event.
         *
         * @param name new name
         * @return the place holder
         */
        static Event placeholderEvent(const std::string& name);

        /**
         * Check whether this Event is a placeholder.
         * @return true if so, false otherwise
         */
        bool isPlaceholder() const;

        /**
         * Check whether all the parameters are undefined
         * @return true if so, false otherwise
         */
        bool empty() const;

        /**
         * Check whether this Event conflicts with the other.
         * two Events conflict if they have the same name but different values, e.g
         * P(Rain=strong | Rain=light)
         * P(Rain=strong | Rain>strong)
         * but not conflicting
         * P(Rain=strong | Rain=strong)
         * @param e the event to check against
         * @return true if *this and e do not conflict, false otherwise
         */
        bool notConflicting(const Event& e) const;

        /**
         * Retrieve the name only.
         * @return the name
         */
        std::string name() const;

        /**
         * Retrieve the value as template-type T_.
         * @return the native value
         */
        template <typename T_>
        T_ value() const
        {
            return toNative<T_>(value_);
        }

        /**
         * Retrieve the value as interval of template-type T_.
         * If the value is indeed a single value, then return a single value interval.
         * @return the type-T_ - interval
         */
        template <typename T_ >
        Interval<T_> interval() const
        {
            if (value_.type() == typeid (Interval<T_>))
                return value_.get<Interval<T_> >();
            else if (value_.type() == typeid (T_))
            {
                T_ v = value_.get<T_>();
                return Interval<T_>(v, v);
            }
            else
                throw event_error(typeid (T_), value_.type());
        }

        /**
         * Retrieve the value as variant.
         * @return the value as variant
         */
        Var varValue() const;

        /**
         * Check whether an Event matches this event considering the name/value/operation.
         * @param e the event to match against
         * @return true if e matches *this, false otherwise
         */
        bool matches(const Event & e) const;


        /**
         * Propagate the operation description to the users of this.
         * @return the operation description
         */
        std::string opDesc() const;

        /**
         * Equality operation needed to enable associative containers.
         * @param lhs left hand side
         * @param rhs right hand side
         * @return true if equal, false otherwise
         */
        friend bool operator==(const Event& lhs, const Event & rhs);

        /**
         * Less than operation needed to enable associative containers.
         * @param lhs left hand side
         * @param rhs right hand side
         * @return true if equal, false otherwise
         */
        friend bool operator<(const Event& lhs, const Event & rhs);

        /**
         * Generic ostream - &lt;&lt; operator for Events.
         * @param os the out-stream
         * @param the right-hand-side event
         * @return the modified stream
         */
        friend std::ostream& operator<<(std::ostream& os, const Event & rhs);
    private:
        std::string name_;
        Var value_;
        Operation* operation_;
        bool isPlaceHolder_;
    };

    typedef std::set<Event> EVENT_SET;

    /**
     * The range that the value of an an Event can assume.
     * This can be a set of disjunct (enumerated) values or a continuous (float)
     * interval.
     */
    class EventValueRange
    {
    public:

        enum DistributionType
        {
            discrete, ///< discrete, enumerated distribution of distinct values
            float_uniform, ///< uniform distribution on a finite floating point interval
            gaussian, ///< gaussian bell distribution on the float range
            exponential ///< exponential distribution on the positive float range
        };
        typedef std::set<Var> RANGEVALUE_SET;

        /**
         * Default construct a boolean range (or not if haveBoolRange == false).
         * @param haveBoolRange whter or not a boolean range should be created
         */
        explicit EventValueRange(bool haveBoolRange = false);

        /**
         * Construct a range of enumerated character values.
         * @param values the set of characters to initialize the range
         */
        EventValueRange(const std::set<VAR_CHAR>& values);

        /**
         * Construct a range of enumerated signed integer values.
         * @param values the set of integrals to initialize the range
         */
        EventValueRange(const std::set<VAR_INT>& values);

        /**
         * Construct a range of enumerated unsigned integer values.
         * @param values the set of unsigned integrals to initialize the range
         */
        EventValueRange(const std::set<VAR_UINT>& values);

        /**
         * Construct a range of enumerated floating point values.
         * @param values the set of floating point to initialize the range
         */
        EventValueRange(const std::set<VAR_FLOAT>& values);

        /**
         * Construct a range of enumerated date values.
         * @param values the set of dates to initialize the range
         */
        EventValueRange(const std::set<VAR_DATE>& values);

        /**
         * Construct a range of enumerated string values.
         * @param values the set of strings to initialize the range
         */
        EventValueRange(const std::set<VAR_STRING>& values);

        /**
         * Construct a range of enumerated character values as an interval from
         * lowest to highest.
         * @param lowest lowest character of the range
         * @param highest highest character of the range
         */
        EventValueRange(VAR_CHAR lowest, VAR_CHAR highest);

        /**
         * Construct a range of enumerated signed integer values as an interval
         * lowest to highest.
         * @param lowest lowest integer of the range
         * @param highest highest integer of the range
         */
        EventValueRange(VAR_INT lowest, VAR_INT highest);

        /**
         * Construct a range of enumerated unsigned integer values as an interval
         * lowest to highest.
         * @param lowest lowest unsigned integer of the range
         * @param highest highest unsigned integer of the range
         */
        EventValueRange(VAR_UINT lowest, VAR_UINT highest);

        /**
         * Construct a range of continuous uniform floating point values as an
         * interval lowest to highest.
         * @param lowest lowest unsigned integer of the range
         * @param highest highest unsigned integer of the range
         */
        EventValueRange(VAR_FLOAT lowest, VAR_FLOAT highest);

        /**
         * Construct a range of continuous uniform floating point values as an
         * interval.
         * @param interval the floating point interval
         */
        EventValueRange(VAR_FLOAT_INTERVAL interval);

        /**
         * Construct a range of continuous for distribution-type.
         * @param tp the type of distribution which defines the range implicitly
         */
        EventValueRange(DistributionType tp);

        /**
         * Check whether the range is empty.
         * @return true if so, false otherwise
         */
        bool empty() const
        {
            return values_.empty();
        }

        /**
         * Populate the enumerated range from a set.
         * @param values a set of values to add to the range
         */
        template<typename T_>
        void setValues(const std::set<T_>& values)
        {
            for (auto it = values.begin(); it != values.end(); it++)
            {
                values_.insert(Var(*it));
            }
        }

        /**
         * Add a value if the type is valid.
         * @param val a potential new value, if type is ok
         * @return true, if value was added, false otherwise
         */
        bool add(const Var& val);

        /**
         * Add a range of values to the range.
         * @param lowest lowest T_ value
         * @param highest highest T_ value
         * true if all value of between lowest and highest were added, false
         *       otherwise
         */
        template<typename T_>
        bool addRange(T_ lowest, T_ highest)
        {
            if (highest < lowest)
                std::swap(highest, lowest);
            bool reval = true;
            for (T_ i = lowest; i <= highest && reval; i++)
                reval &= insert(i);

            return reval;
        }

        /**
         * Add a range of values to the range specialised for floating point
         * (continuous) range.
         * @param lowest lowest floating point value
         * @param highest highest floating point value
         * true if all value of between lowest and highest were added, false
         *       otherwise
         */
        bool addRange(VAR_FLOAT lowest, VAR_FLOAT highest)
        {
            if (highest < lowest)
                std::swap(highest, lowest);
            bool reval = highest != lowest;
            values_.clear();
            insert(lowest);
            insert(highest);

            return reval;
        }

        /**
         * Get the size of the range. Continuous ranges are always 0 or 2
         * (lowest,highest)
         * @return the size of the range
         */
        size_t size() const;

        /**
         * Check whether the range is continuous
         * @return true, if so, false otherwise
         */
        bool isContinuous() const;

        /**
         * Retrieve the type of distribution for this range.
         * @return the type of distribution
         */
        DistributionType type() const;

        /**
         * Check whether a value is in range.
         * @param value the value to check
         * @return true if so, false otherwise
         */
        bool validValue(const Var& value) const;

        /**
         * Check whether a value has the correct type for this range.
         * If no value has been added to the range yet then any Var is valid
         * otherwise all have to be of the same type.
         * @param value the value to check
         * @return true if so, false otherwise
         */
        bool validType(const Var& value) const;

        /**
         * Create a list of Events Event(name,x) where x is in the range
         * by conjugating the name with all possible values.
         * Helper for the creation of canonised (full) probability table.
         * @param name name of the new event
         * @return the created list of events
         */
        EVENT_SET makeEventSet(const std::string& name) const;

        /**
         * Generic ostream - &lt;&lt; operator for EventValueRange.
         * @param os the ostream
         * @param evr the event value range to write on the stream
         * @return the modified stream
         */
        friend std::ostream& operator<<(std::ostream& os, const EventValueRange& evr);
    private:

        /**
         * Template helper to ensure only Var's of the same type can be inserted
         * into the collection.
         * @param v the value to insert
         * @return true, it insertion successful, false otherwise
         */
        template<typename T_>
        bool insert(const T_& v)
        {
            bool reval = false;
            if (values_.empty() || sameType(v, *(values_.begin())))
            {
                values_.insert(v);
                reval = true;
            }

            return reval;
        }
        DistributionType type_; ///< Indicates the type range.
        RANGEVALUE_SET values_; ///< Collection describing the range.
    };

    /**
     * Tag for a list of statistical Events (Event_1 and Event_2 and ...).
     * This enables statistical expressions like P(E1=e1,E2 &lt; e2,...)
     */
    class EventList
    {
    public:
        typedef std::set<Event> EVENT_CONTAINER;
        typedef EVENT_CONTAINER::iterator EVENT_CONTAINER_ITER;
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Default onstruct empty event-list.
         */
        EventList();
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Construct a one-element event-list (if Event is not empty).
         */
        EventList(const Event& e);
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Copy construct an EventList.
         */
        EventList(const EventList& rhs);
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Assign the right-hand-side to this.
         */
        EventList& operator=(const EventList& rhs);
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Append a single Event to the this.
         */
        EventList& operator&&(const Event& e);
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Append a list of Events to this.
         */
        EventList& operator&&(const EventList& el);
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Check for emptiness.
         */
        bool empty() const;
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Get the number of Events in the list.
         */
        size_t size() const;
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Check whether two lists are not conflicting (using the Event-definition of conflict).
         */
        bool notConflicting(const EventList& eList) const;
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Check whether this list matches the other EventList.
         */
        bool matches(const EventList& eList) const;
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Retrieve an event from this list by name.
         */
        Event eventByName(const std::string& name) const;
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Move name-specified event from this list to the other.
         */
        bool moveEvent(const std::string& name, EventList& el);
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Check whether an event named name is in this list.
         */
        bool hasEvent(const std::string& e) const;
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Iterator pointing to the start of the list of events.
         */
        EVENT_CONTAINER_ITER begin();
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Iterator pointing to the start of the list of events (constant version).
         */
        EVENT_CONTAINER_CITER cbegin() const;
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * terator pointing to the end of the list of events.
         */
        EVENT_CONTAINER_ITER end();
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Iterator pointing to the end of the list of events (constant version).
         */
        EVENT_CONTAINER_CITER cend() const;
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Remove the Event pointed to by the iterator from this list.
         */
        void erase(EVENT_CONTAINER_ITER it);
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Enable associative containers.
         */
        friend bool operator==(const EventList& lhs, const EventList& rhs);
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Enable associative containers.
         */
        friend bool operator<(const EventList& lhs, const EventList& rhs);
        typedef EVENT_CONTAINER::const_iterator EVENT_CONTAINER_CITER;

        /**
         * Generic ostream - &lh;&lt; operator for EventList.
         */
        friend std::ostream& operator<<(std::ostream&, const EventList& eList);

    private:
        EVENT_CONTAINER evts_; ///< Ordered list of Events.
    };

    /**
     * Global operator that creates an EventList from two Events.
     * @praram lhs left-hand-side
     * @praram rhs right-hand-side
     * @return the newly created list of events
     */
    EventList operator&&(const Event& lhs, const Event& rhs);

    /**
     * Tag for a list of statistical conditional Events.
     * his enables statistical expressions like P(E1=e1,E2\<e2|E3\>e3,E4 in [0..11])
     */
    class CondEvent
    {
    public:
        typedef std::deque<CondEvent> CONDEVENT_LIST;
        typedef CONDEVENT_LIST::iterator CONDEVENT_LIST_ITER;
        typedef CONDEVENT_LIST::const_iterator CONDEVENT_LIST_CITER;

        /**
         * Default construct from two event-lists: first for the event, second
         *  for the condition.
         *
         * @param e an event list
         * @param cond a condition list
         */
        CondEvent(const EventList& e = EventList(), const EventList& cond = EventList());

        /**
         * Construct from a csv-object, more precisely from the column- headers and types.
         * 
         */
        CondEvent(const CSVAnalyzer& csv,
                  size_t row = 0,
                  size_t lastEventIndex = 0,
                  bool isAccumulativeCSV = false);

        /**
         * Copy-construct CondEvent.
         */
        CondEvent(const CondEvent& rhs);

        /**
         * Assign right-hand-side to this.
         */
        CondEvent& operator=(const CondEvent& rhs);

        /**
         * Check for emptyness.
         */
        bool empty() const;

        /**
         * Retrieve the number of events (not conditions) in this CondEvent.
         */
        size_t eventSize() const;

        /**
         * Retrieve the number of conditions (not events) in this CondEvent.
         */
        size_t conditionSize() const;

        /**
         * Retrieve the event-part.
         */
        const EventList& event() const;

        /**
         * retrieve the condition-part.
         */
        const EventList& condition() const;

        /**
         * Filter out conditions with names in the list conds.
         */
        CondEvent filterConditions(const std::set<std::string>& conds) const;

        /**
         * Check whether the right-hand-side is a match to this.
         */
        bool isMatch(const CondEvent& ce) const;

        /**
         * Check whether the condition-part contains an event named name.
         */
        bool containsCondition(const std::string& name) const;

        /**
         * Apply the chain rule of probability. Result in referenced parameter.
            bool chainRule(CONDEVENT_LIST& cel, const std::string& name) const;

        /**
         * Apply the chain rule of probability. Result in referenced parameter.
         */
        bool chainRule(CONDEVENT_LIST& cel, const std::vector<std::string>& nameList) const;

        /**
         * Append an event to event-part.
         */
        CondEvent& operator&&(const Event& el);

        /**
         * append an event to the condition-part.
         */
        CondEvent& operator||(const Event& el);

        /**
         * Check whether the event-part contains an Event named name.
         */
        bool hasEvent(const std::string& el) const;

        /**
         * Check whether the condition-part contains an Event named name.
         */
        bool hasCondition(const std::string& el) const;

        /**
         * Enable associative containers.
         */
        friend bool operator==(const CondEvent& lhs, const CondEvent& rhs);

        /**
         * Enable associative containers.
         */
        friend bool operator<(const CondEvent& lhs, const CondEvent& rhs);

        /**
         * Generic ostream - &lh;&lt; operator for CondEvent.
         */
        friend std::ostream& operator<<(std::ostream&, const CondEvent& eList);
    protected:

        /**
         * Retrieve the condition named name.
         */
        Event* getCondition(const std::string& name) const;


    private:
        EventList eList_; ///< List of Events interpreted as events.
        EventList condList_; ///< List of Events interpreted as conditions.
    };

    /// Global operator that creates a Condition Event from two EventLists.
    CondEvent operator||(const EventList& lhs, const EventList& rhs);

    const static long double e = exp(1.0L); ///< Base of the natural logarithm.
    const static long double ln_2 = log(2.0L); ///< Natural logarithm of 2.

    typedef std::map<std::string, EventValueRange> VALUERANGES_TYPE;
    typedef VALUERANGES_TYPE::iterator VALUERANGES_TYPE_ITER;
    typedef VALUERANGES_TYPE::const_iterator VALUERANGES_TYPE_CITER;

    /// Abstract base class for floating point probability functions.

    struct ProbabilityFunction
    {
        /// Default construct.
        ProbabilityFunction(const VALUERANGES_TYPE& eventValueRanges = VALUERANGES_TYPE(),
                            const VALUERANGES_TYPE& conditionValueRanges = VALUERANGES_TYPE());

        /// Clone a copy of this.
        virtual ProbabilityFunction* clone() const = 0;
        /// Check whether a condition-event is compatible with this probability function.
        bool possibleCondEvent(const CondEvent& ce, std::string& error) const;
        /// Probability of a conditional event.
        virtual long double P(const CondEvent& ce) const = 0;
        /// Delegate Probability from EventList to avoid excessive casting.
        virtual long double P(const EventList& el) const = 0;
        /// Does the function satisfy probability requirements?

        virtual bool isDistribution() const
        {
            return true;
        }
        /// Reset the parameters of the probability function.
        virtual void clear() = 0;
        /// Train (estimate) the parameters of of the probability function.
        virtual bool train(CSVAnalyzer csv,
                           bool isAccumulativeCSV = false) = 0;

        /// Add a Variant-value to the range of possible event-values.
        bool addValueToEventRange(const std::string& name, const Var& val);
        /// Add a Variant-value to the range of possible condition-values.
        bool addValueToConditionRange(const std::string& name, const Var& val);

    protected:
        /// Get the index up to which we interpret as event-Events (as opposed to
        /// condition-Events). Needed when we have a list like we get it from a csv.
        size_t getLastEventIndex() const;
        /// Number of Conditions. Use in combination with getLastEventIndex() to
        /// split a yet un-discriminated list into event and condition parts.
        size_t getNumberOfConditions() const;
        /// Add a value to a range (either event or condition). Vets against
        /// nonsensical inserts.
        bool addValidValueToRange(VALUERANGES_TYPE& range,
                                  VALUERANGES_TYPE& range_ortho,
                                  const std::string& name,
                                  const Var& value);
        VALUERANGES_TYPE eventValueRanges_; ///< Map of ranges for events.
        VALUERANGES_TYPE conditionValueRanges_; ///< Map of ranges for conditions.
    };

    /// Uniform probability function (on real domain).

    class UniformFloatFunction : public ProbabilityFunction
    {
    public:
        /// Storage for a set of parameters for a Uniform function (min/max).

        struct UNIF_PARAM
        {

            UNIF_PARAM(VAR_FLOAT l = 0.0L, VAR_FLOAT h = 1.0L)
            : low(l)
            , high(h)
            , occurences(0.0L)
            {
            }
            long double low, high, occurences;
        };
        typedef std::map<EventList, UNIF_PARAM> UNIF_PARAM_TABLE;
        typedef UNIF_PARAM_TABLE::iterator UNIF_PARAM_TABLE_ITER;
        typedef UNIF_PARAM_TABLE::const_iterator UNIF_PARAM_TABLE_CITER;
        /// Default construct.
        UniformFloatFunction(VAR_FLOAT minVal = 0.0L,
                             VAR_FLOAT maxVal = 1.0L,
                             const VALUERANGES_TYPE& conditionValueRanges = VALUERANGES_TYPE());

        /// Clone a copy of this.
        ProbabilityFunction* clone() const;
        /// Probability of an interval. Returns zero if the Event is not an interval-event.
        long double P(const CondEvent& ce) const;
        /// Delegate Probability from EventList to avoid excessive casting.
        long double P(const EventList& el) const;
        /// Reset the parameters of the probability function.
        void clear();
        /// Train (estimate) the parameters of of the probability function.
        bool train(CSVAnalyzer csv, bool isAccumulativeCSV = false);
        UNIF_PARAM_TABLE param_; ///< Maps conditions to min-max-values.

        /// Generic ostream - &lh;&lt; operator for UniformFloatFunction.
        friend std::ostream& operator<<(std::ostream& os, const UniformFloatFunction& d);

    };

    /// Gaussian bell curve probability function.

    class GaussFunction : public ProbabilityFunction
    {
    public:
        /// Storage for a set of parameters for a Gaussian function.

        struct GAUSS_PARAM
        {

            GAUSS_PARAM(VAR_FLOAT m = 0.0L, VAR_FLOAT s = 0.0L)
            : mu(m)
            , sigma(s)
            , occurences(0.0L)
            {
            }
            long double mu, sigma, occurences;
        };
        typedef std::map<EventList, GAUSS_PARAM> GAUSS_PARAM_TABLE;
        typedef GAUSS_PARAM_TABLE::iterator GAUSS_PARAM_TABLE_ITER;
        typedef GAUSS_PARAM_TABLE::const_iterator GAUSS_PARAM_TABLE_CITER;

        /// Default construct with expectation mu and variance sigma
        GaussFunction(long double mu = 0.0L, long double sigma = 1.0L);

        /// Clone a copy of this.

        ProbabilityFunction* clone() const
        {
            return new GaussFunction(*this);
        }
        /// Reset the parameters mu and sigma to standard normal values.
        void clear();
        /// Probability of an interval. Returns zero if the Event is not an interval-event.
        long double P(const CondEvent& ce) const;
        /// Delegate Probability from EventList to avoid excessive casting.
        long double P(const EventList& el) const;

        /// Estimate mu and sigma.
        /// - mu ~ sum(x)/numberOf(x)
        /// - sigma ~ sum((x-mu)^2\)/numberOf(x)
        bool train(CSVAnalyzer csv, bool isAccumulativeCSV);

        /// Retrieve mu.
        long double mu(const CondEvent& ce = CondEvent()) const;
        /// Retrieve variance.
        long double sigma(const CondEvent& ce = CondEvent()) const;

        /// Generic ostream - &lh;&lt; operator for GaussFunction.
        friend std::ostream& operator<<(std::ostream& os, const GaussFunction& d);

    private:
        GAUSS_PARAM_TABLE param_; ///< Maps conditions to Gauss-parameters.
    };

    /// Exponential probability function.

    class ExponentialFunction : public ProbabilityFunction
    {
    public:
        /// Storage for a set of parameters for an exponential function.

        struct EXP_PARAM
        {

            EXP_PARAM(long double l = 1.0L)
            : lambda(l)
            , occurences(0.0L)
            {
            }
            long double lambda;
            long double occurences;
        };
        typedef std::map<EventList, EXP_PARAM> EXP_PARAM_TABLE;
        typedef EXP_PARAM_TABLE::iterator EXP_PARAM_TABLE_ITER;
        typedef EXP_PARAM_TABLE::const_iterator EXP_PARAM_TABLE_CITER;

        /// Default construct with expectation lambda.
        ExponentialFunction(VAR_FLOAT lambda = 1.0L);

        /// Clone a copy of this.

        ProbabilityFunction* clone() const
        {
            return new ExponentialFunction(*this);
        }
        /// Reset the expectation to 1.0.
        void clear();
        /// Probability of an interval. Returns zero if the Event is not an interval-event.
        long double P(const CondEvent& ce) const;

        /// Delegate Probability from EventList to avoid excessive casting.
        long double P(const EventList& el) const;

        /// Estimate lambda for first column (must be float). Last column might be
        /// an accumulative column - all others are condition.
        /// - lambda ~ sum(x)/numberOf(x)
        bool train(CSVAnalyzer csv, bool isAccumulativeCSV);
        /// Retrieve the expectation lambda.
        long double lambda(const CondEvent& ce = CondEvent()) const;

        /// Get the point where the cdf == 1/2.
        /// - P(0.0\<= x \<= ln(2)/lambda_) = 0.5
        long double ln2ByLambda(const CondEvent& ce = CondEvent()) const;

        /// Generic ostream - &lh;&lt; operator for ExponentialFunction.
        friend std::ostream& operator<<(std::ostream& os, const ExponentialFunction& d);

    private:
        EXP_PARAM_TABLE param_; ///< Maps conditions to Exponential-parameters.
    };

    /// Helper for accumulation of discrete probability table.
    /// Keeps track of how many values n where used to create sum s.

    struct ACCUMULATION_DATA
    {
        /// Construct with initial sum = 0.0 and number = 0

        ACCUMULATION_DATA(long double s = 0L, long double n = 0L)
        : sum(s)
        , number(n)
        {
        }
        long double sum, number;
    };
    typedef std::map<EventList, ACCUMULATION_DATA> ACCUMULATION_MAP;

    /// Discrete probability function that enumerates value-probability-pairs.

    class DiscreteProbability : public ProbabilityFunction
    {
    public:
        typedef std::map<CondEvent, long double> PROB_TABLE;
        typedef PROB_TABLE::iterator PROB_TABLE_ITER;
        typedef PROB_TABLE::const_iterator PROB_TABLE_CITER;

        /// Default construct using event and condition value ranges.
        DiscreteProbability(const VALUERANGES_TYPE& eventValueRanges = VALUERANGES_TYPE(),
                            const VALUERANGES_TYPE& conditionValueRanges = VALUERANGES_TYPE());
        /// Clone a copy of this.
        ProbabilityFunction* clone() const;

        /// Set an equal probability for every event-value.
        bool makeUniform();
        /// Make sure that probability values add up to 1.0 .
        bool normalise();
        /// Add probability values for all possible event-value combinations.
        bool canonise();
        /// Check whether the probabilities add up to 1.0 .
        bool isDistribution() const;
        /// Reset the distribution.
        void clear();
        /// Check whether the distribution is empty.
        bool empty() const;

        /// Probability of an interval. Returns probability of single value if
        /// interval is single value.
        virtual long double P(const CondEvent& ce) const;
        /// Delegate Probability from EventList to avoid excessive casting.

        long double P(const EventList& el) const
        {
            return P(CondEvent(el));
        }

        /// Estimate the probability function using a csv.
        /// - lastEventIndex is the last column that is not a condition
        /// - if lastEventIndex ==  x  columns x+1, x+2, x+3, ... are conditions
        virtual bool train(CSVAnalyzer csv, bool isAccumulativeCSV = false);

        /// Generic ostream - &lh;&lt; operator for DiscreteProbability.
        friend std::ostream& operator<<(std::ostream& os, const DiscreteProbability& d);
        ///

        void resetDistribution()
        {
            isUniform_ = false;
            hasBeenModified_ = false;
            clear();
        }
        /// Return true if the distribution values have uniform probability.

        bool isUniform() const
        {
            return isUniform_;
        }
        /// Check whether the distribution table has been modified.

        bool isModified() const
        {
            return hasBeenModified_;
        }

    protected:
        /// Use the values (obtained by training) to update the ranges the variables
        /// can assume. Don't remove any existing values.
        void updateValueRangesFromValues_(bool clearFirst = false);
        /// Set the distribution to uniform. Don't update the table values.

        void setUniform(bool uni)
        {
            isUniform_ = uni;
        }
        /// Set the state to modified to indicate to users that normalisation
        /// has to happen before probability calculations can take place.

        void setModified(bool mod) const
        {
            hasBeenModified_ = mod;
        }

    private:
        bool isUniform_;
        mutable bool hasBeenModified_;
        PROB_TABLE values_;
    };
}; // namespace util

#endif //NS_UTIL_STATUTIL_H_INCLUDED
