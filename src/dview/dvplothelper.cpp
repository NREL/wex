/***********************************************************************************************************************
*  WEX, Copyright (c) 2008-2017, Alliance for Sustainable Energy, LLC. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
*  following conditions are met:
*
*  (1) Redistributions of source code must retain the above copyright notice, this list of conditions and the following
*  disclaimer.
*
*  (2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
*  following disclaimer in the documentation and/or other materials provided with the distribution.
*
*  (3) Neither the name of the copyright holder nor the names of any contributors may be used to endorse or promote
*  products derived from this software without specific prior written permission from the respective party.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
*  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES GOVERNMENT, OR ANY CONTRIBUTORS BE LIABLE FOR
*  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
*  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
*  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************************************/

#include "wex/dview/dvplothelper.h"

namespace wxDVPlotHelper
{
	void MouseWheelZoom(double* worldMin, double* worldMax, wxCoord center, wxCoord physMin, wxCoord physMax, int step)
	{
		//Center zooming on the location of the mouse.
		center -= physMin;
		physMax -= physMin;

		double locationPercent = double(center) / double(physMax);
		locationPercent -= 0.5;

		if (step > 0)
		{
			ZoomFactor(worldMin, worldMax, 1.5 * step, locationPercent);
		}
		else
		{
			ZoomFactor(worldMin, worldMax, 1 / (-1.5 * step));
		}
	}

	void ZoomFactor(double* worldMin, double* worldMax, double factor, double shiftPercent)
	{
		//A factor of 2 would zoom in twice as far as current level.
		if (factor == 1)
			return;

		double oldRange = *worldMax - *worldMin;
		double newRange = oldRange / factor;
		double newMin = *worldMin + (oldRange - newRange) / 2.0;
		double newMax = *worldMax - (oldRange - newRange) / 2.0;

		newMin += shiftPercent * (newRange < oldRange ? newRange : oldRange);
		newMax += shiftPercent * (newRange < oldRange ? newRange : oldRange);

		*worldMin = newMin;
		*worldMax = newMax;
	}

	void SetRangeEndpointsToDays(double* min, double* max)
	{
		//This function doesn't really set endpoints to days if the range is sufficiently small.
		//Choose an appropriate interval based on range.
		int intMin = int(*min);
		int intMax = int(*max);

		int range = intMax - intMin;
		int interval; //hours to set endpoint to
		if (range <= 6)
			return;
		else if (range <= 12)
			interval = 3;
		else if (range <= 24)
			interval = 3; //Set endpoints to eighth-days.
		else if (range <= 48)
			interval = 12;
		else
			interval = 24; //Actually set endpoints to days.

		int oldIntMin = intMin;
		if (intMin % interval > interval / 2 && intMin / interval != intMax / interval)
			intMin += interval;
		intMin -= intMin % interval;

		if (intMax % interval > interval / 2 || oldIntMin / interval == intMax / interval)
			intMax += interval;
		intMax -= intMax % interval;

		*min = double(intMin);
		*max = double(intMax);
	}
}