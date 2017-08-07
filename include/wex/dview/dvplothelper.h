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

#ifndef __DVPlotHelper_h
#define __DVPlotHelper_h

/*
 * This class just contains some helper functions that may be useful in several different places in the code.
 */

#include <wx/wx.h>

#include <vector>

namespace wxDVPlotHelper
{
	//static int recursionDepth;

	void ZoomFactor(double* worldMin, double* worldMax, double factor, double shiftPercent = 0);
	void MouseWheelZoom(double* worldMin, double* worldMax, wxCoord center, wxCoord physMin, wxCoord physMax, int step);

	void SetRangeEndpointsToDays(double* min, double* max);
	void RoundToNearest(double* numToRound, const double interval);
	void RoundUpToNearest(double* numToRound, const double interval);
	void RoundDownToNearest(double* numToRound, const double interval);

	template <typename T> void Swap(T* a, T* b);
	template <typename T> void BubbleSort(std::vector<T>* data); //Must have > defined.

	template <typename T> void SelectionSort(std::vector<T> &p);

	template <typename T> void QuickSort(std::vector<T>* data, int left, int right);
	template <typename T> void QuickSort(std::vector<T>* data);

	//*** TEMPLATED SORTING ALGORITHMS ***//
	template <typename T>
	void Swap(T* a, T* b)
	{
		T temp = *a;
		*a = *b;
		*b = temp;
	}

	template <typename T>
	void BubbleSort(std::vector<T>* data)
	{
		int n = data->length();
		do
		{
			int newN = 0;
			for (int i = 0; i < n - 1; i++)
			{
				if (data->at(i) > data->at(i + 1))
				{
					Swap(&(data->at(i)), &(data->at(i + 1)));
					newN = i + 1;
				}
			}
			n = newN;
		} while (n > 1);
	}

	template<typename T>
	void SelectionSort(std::vector<T> &p)
	{
		int n = p.size();
		for (int i = 0; i < n - 1; i++)
		{
			int smallest = i;
			for (int j = i + 1; j < n; j++)
				if (p[j] < p[smallest])
					smallest = j;

			// swap
			T temp = p[i];
			p[i] = p[smallest];
			p[smallest] = temp;
		}
	}

	//This crashes if you have many items that are equal in your array.
	template <typename T>
	void QuickSort(std::vector<T>* data, int left, int right)
	{
		int i, j, pivot, temp;

		if (left == right) return;
		i = left;
		j = right;
		pivot = data->at((left + right) / 2);

		/* Split the array into two parts */
		do
		{
			while (data->at(i) < pivot) i++;
			while (data->at(j) > pivot) j--;
			if (i <= j)
			{
				temp = data->at(i);
				data->at(i) = data->at(j);
				data->at(j) = temp;
				i++;
				j--;
			}
		} while (i <= j);

		if (left < j) QuickSort(data, left, j);
		if (i < right) QuickSort(data, i, right);
	}

	template <typename T>
	void QuickSort(std::vector<T>* data)
	{
		QuickSort(data, 0, data->length() - 1);
	}
}

#endif
