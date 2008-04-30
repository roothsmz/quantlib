/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "creditdefaultswap.hpp"
#include "utilities.hpp"
#include <ql/instruments/creditdefaultswap.hpp>
#include <ql/pricingengines/credit/midpointcdsengine.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/termstructures/credit/interpolatedhazardratecurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/discountcurve.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <iomanip>
#include <iostream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void CreditDefaultSwapTest::testCachedValue() {

    BOOST_MESSAGE("Testing credit-default swap against cached values...");

    SavedSettings backup;

    // Initialize curves
    Settings::instance().evaluationDate() = Date(9,June,2006);
    Date today = Settings::instance().evaluationDate();

    Handle<Quote> hazardRate = Handle<Quote>(
                boost::shared_ptr<Quote>(new SimpleQuote(0.01234)));
    RelinkableHandle<DefaultProbabilityTermStructure> probabilityCurve;
    probabilityCurve.linkTo(
        boost::shared_ptr<DefaultProbabilityTermStructure>(
            new FlatHazardRate(hazardRate, Actual360())));

    RelinkableHandle<YieldTermStructure> discountCurve;

    discountCurve.linkTo(boost::shared_ptr<YieldTermStructure>(
                            new FlatForward(today,0.06,Actual360())));

    // Build the schedule
    Calendar calendar = TARGET();
    Date issueDate = calendar.advance(today, -1, Years);
    Date maturity = calendar.advance(issueDate, 10, Years);
    Frequency frequency = Semiannual;
    BusinessDayConvention convention = ModifiedFollowing;

    Schedule schedule(issueDate, maturity, Period(frequency), calendar,
                      convention, convention, DateGeneration::Forward, false);

    // Build the CDS
    Rate fixedRate=0.0120;
    DayCounter dayCount=Actual360();
    Real notional=10000.0;
    Real recoveryRatio = 0.4;
    Issuer issuer(probabilityCurve, recoveryRatio);

    CreditDefaultSwap cds(Protection::Seller, notional, fixedRate,
                          schedule, convention, dayCount, true, true);
    cds.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                new MidPointCdsEngine(issuer,discountCurve)));

    double calculatedNpv = cds.NPV();
    double calculatedFairRate = cds.fairSpread();
    double npv = 295.0153398, fairRate=0.007517539081;
    double tolerance = 1.0e-7;

    if (std::fabs(calculatedNpv - npv) > tolerance)
        BOOST_ERROR(
            "Failed to reproduce cds NPV\n"
            << std::setprecision(10)
            << "    calculated NPV: " << calculatedNpv << "\n"
            << "    expected NPV:   " << npv);

    if (std::fabs(calculatedFairRate - fairRate) > tolerance)
        BOOST_ERROR(
            "Failed to reproduce cds fair rate\n"
            << std::setprecision(10)
            << "    calculated fair rate: " << calculatedFairRate << "\n"
            << "    expected fair rate:   " << fairRate);
}


void CreditDefaultSwapTest::testCachedMarketValue() {

    BOOST_MESSAGE(
        "Testing credit-default swap against cached market values...");

    SavedSettings backup;

    Settings::instance().evaluationDate() = Date(9,June,2006);
    Date evalDate = Settings::instance().evaluationDate();
    Calendar calendar = UnitedStates();

    std::vector<Date> discountDates;
    discountDates.push_back(evalDate);
    discountDates.push_back(calendar.advance(evalDate, 1, Weeks,  ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 1, Months, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 2, Months, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 3, Months, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 6, Months, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate,12, Months, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 2, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 3, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 4, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 5, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 6, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 7, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 8, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 9, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate,10, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate,15, Years, ModifiedFollowing));

    std::vector<DiscountFactor> dfs;
    dfs.push_back(1.0);
    dfs.push_back(0.9990151375768731);
    dfs.push_back(0.99570502636871183);
    dfs.push_back(0.99118260474528685);
    dfs.push_back(0.98661167950906203);
    dfs.push_back(0.9732592953359388 );
    dfs.push_back(0.94724424481038083);
    dfs.push_back(0.89844996737120875  );
    dfs.push_back(0.85216647839921411  );
    dfs.push_back(0.80775477692556874  );
    dfs.push_back(0.76517289234200347  );
    dfs.push_back(0.72401019553182933  );
    dfs.push_back(0.68503909569219212  );
    dfs.push_back(0.64797499814013748  );
    dfs.push_back(0.61263171936255534  );
    dfs.push_back(0.5791942350748791   );
    dfs.push_back(0.43518868769953606  );

    const DayCounter& curveDayCounter=Actual360();

    RelinkableHandle<YieldTermStructure> discountCurve;
    discountCurve.linkTo(
        boost::shared_ptr<YieldTermStructure>(
            new DiscountCurve(discountDates, dfs, curveDayCounter)));

    DayCounter dayCounter = Thirty360();
    std::vector<Date> dates;
    dates.push_back(evalDate);
    dates.push_back(calendar.advance(evalDate, 6, Months, ModifiedFollowing));
    dates.push_back(calendar.advance(evalDate, 1, Years, ModifiedFollowing));
    dates.push_back(calendar.advance(evalDate, 2, Years, ModifiedFollowing));
    dates.push_back(calendar.advance(evalDate, 3, Years, ModifiedFollowing));
    dates.push_back(calendar.advance(evalDate, 4, Years, ModifiedFollowing));
    dates.push_back(calendar.advance(evalDate, 5, Years, ModifiedFollowing));
    dates.push_back(calendar.advance(evalDate, 7, Years, ModifiedFollowing));
    dates.push_back(calendar.advance(evalDate,10, Years, ModifiedFollowing));

    std::vector<Probability> defaultProbabilities;
    defaultProbabilities.push_back(0.0000);
    defaultProbabilities.push_back(0.0047);
    defaultProbabilities.push_back(0.0093);
    defaultProbabilities.push_back(0.0286);
    defaultProbabilities.push_back(0.0619);
    defaultProbabilities.push_back(0.0953);
    defaultProbabilities.push_back(0.1508);
    defaultProbabilities.push_back(0.2288);
    defaultProbabilities.push_back(0.3666);

    std::vector<Real> hazardRates;
    hazardRates.push_back(0.0);
    for (Size i=1; i<dates.size(); ++i) {
        Time t1 = dayCounter.yearFraction(dates[0], dates[i-1]);
        Time t2 = dayCounter.yearFraction(dates[0], dates[i]);
        Probability S1 = 1.0 - defaultProbabilities[i-1];
        Probability S2 = 1.0 - defaultProbabilities[i];
        hazardRates.push_back(std::log(S1/S2)/(t2-t1));
    }

    RelinkableHandle<DefaultProbabilityTermStructure> piecewiseFlatHazardRate;
    piecewiseFlatHazardRate.linkTo(
        boost::shared_ptr<DefaultProbabilityTermStructure>(
               new InterpolatedHazardRateCurve<BackwardFlat>(dates,
                                                             hazardRates,
                                                             Thirty360())));

    // Testing credit default swap

    // Build the schedule
    Date issueDate(20, March, 2006);
    Date maturity(20, June, 2013);
    Frequency cdsFrequency = Semiannual;
    BusinessDayConvention cdsConvention = ModifiedFollowing;

    Schedule schedule(issueDate, maturity, Period(cdsFrequency), calendar,
                      cdsConvention, cdsConvention,
                      DateGeneration::Forward, false);

    // Build the CDS
    Real recoveryRatio = 0.25;
    Rate fixedRate=0.0224;
    DayCounter dayCount=Actual360();
    Real cdsNotional=100.0;
    Issuer issuer(piecewiseFlatHazardRate, recoveryRatio);

    CreditDefaultSwap cds(Protection::Seller, cdsNotional, fixedRate,
                          schedule, cdsConvention, dayCount, true, true);
    cds.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                new MidPointCdsEngine(issuer,discountCurve)));

    double calculatedNpv = cds.NPV();
    double calculatedFairRate = cds.fairSpread();

    double npv = -1.364048777;        // from Bloomberg we have 98.15598868 - 100.00;
    double fairRate =  0.0248429452; // from Bloomberg we have 0.0258378;

    Real tolerance = 1e-9;

    if (std::fabs(npv - calculatedNpv) > tolerance)
        BOOST_ERROR(
            "Failed to reproduce the npv for the given credit-default swap\n"
            << std::setprecision(10)
            << "    computed NPV:  " << calculatedNpv << "\n"
            << "    Given NPV:     " << npv);

    if (std::fabs(fairRate - calculatedFairRate) > tolerance)
        BOOST_ERROR(
            "Failed to reproduce the fair rate for the given credit-default swap\n"
            << std::setprecision(10)
            << "    computed fair rate:  " << calculatedFairRate << "\n"
            << "    Given fair rate:     " << fairRate);
}


void CreditDefaultSwapTest::testImpliedHazardRate() {

    BOOST_MESSAGE("Testing implied hazard-rate for credit-default swaps...");

    SavedSettings backup;

    // Initialize curves
    Date today = Settings::instance().evaluationDate();

    Rate h1 = 0.30, h2 = 0.40;
    DayCounter dayCounter = Actual365Fixed();

    std::vector<Date> dates(3);
    std::vector<Real> hazardRates(3);
    dates[0] = today;
    hazardRates[0] = h1;

    dates[1] = today + 5*Years;
    hazardRates[1] = h1;

    dates[2] = today + 10*Years;
    hazardRates[2] = h2;

    RelinkableHandle<DefaultProbabilityTermStructure> probabilityCurve;
    probabilityCurve.linkTo(boost::shared_ptr<DefaultProbabilityTermStructure>(
                    new InterpolatedHazardRateCurve<BackwardFlat>(dates,
                                                                  hazardRates,
                                                                  dayCounter)));

    RelinkableHandle<YieldTermStructure> discountCurve;
    discountCurve.linkTo(boost::shared_ptr<YieldTermStructure>(
                            new FlatForward(today,0.03,Actual360())));


    Calendar calendar = TARGET();
    Frequency frequency = Semiannual;
    BusinessDayConvention convention = ModifiedFollowing;

    Date issueDate = calendar.advance(today, -6, Months);
    Rate fixedRate = 0.0120;
    DayCounter cdsDayCount = Actual360();
    Real notional = 10000.0;
    Real recoveryRatio = 0.4;
    Issuer issuer(probabilityCurve, recoveryRatio);

    Rate latestRate = Null<Rate>();
    for (Integer n=6; n<=10; ++n) {

        Date maturity = calendar.advance(issueDate, n, Years);
        Schedule schedule(issueDate, maturity, Period(frequency), calendar,
                          convention, convention,
                          DateGeneration::Forward, false);

        CreditDefaultSwap cds(Protection::Seller, notional, fixedRate,
                              schedule, convention, cdsDayCount,
                              true, true);
        cds.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                new MidPointCdsEngine(issuer,discountCurve)));

        Real NPV = cds.NPV();
        Rate flatRate = cds.impliedHazardRate(NPV, discountCurve,
                                              dayCounter,
                                              issuer.recoveryRate());

        if (flatRate < h1 || flatRate > h2) {
            BOOST_ERROR("implied hazard rate outside expected range\n"
                        << "    maturity: " << n << " years\n"
                        << "    expected minimum: " << h1 << "\n"
                        << "    expected maximum: " << h2 << "\n"
                        << "    implied rate:     " << flatRate);
        }

        if (n > 6 && flatRate < latestRate) {
            BOOST_ERROR("implied hazard rate decreasing with swap maturity\n"
                        << "    maturity: " << n << " years\n"
                        << "    previous rate: " << latestRate << "\n"
                        << "    implied rate:  " << flatRate);
        }

        latestRate = flatRate;

        RelinkableHandle<DefaultProbabilityTermStructure> probability;
        probability.linkTo(boost::shared_ptr<DefaultProbabilityTermStructure>(
         new FlatHazardRate(
           Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(flatRate))),
           dayCounter)));
        Issuer i(probability, issuer.recoveryRate());

        CreditDefaultSwap cds2(Protection::Seller, notional, fixedRate,
                               schedule, convention, cdsDayCount,
                               true, true);
        cds2.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                new MidPointCdsEngine(issuer,discountCurve)));

        Real NPV2 = cds2.NPV();
        Real tolerance = 1.0;
        if (std::fabs(NPV-NPV2) > tolerance) {
            BOOST_ERROR("failed to reproduce NPV with implied rate\n"
                        << "    expected:   " << NPV << "\n"
                        << "    calculated: " << NPV2);
        }
    }
}


test_suite* CreditDefaultSwapTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Credit-default swap tests");
    suite->add(BOOST_TEST_CASE(&CreditDefaultSwapTest::testCachedValue));
    suite->add(BOOST_TEST_CASE(&CreditDefaultSwapTest::testCachedMarketValue));
    suite->add(BOOST_TEST_CASE(&CreditDefaultSwapTest::testImpliedHazardRate));
    return suite;
}
