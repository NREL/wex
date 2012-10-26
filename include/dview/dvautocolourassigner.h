#ifndef __DVAutoColourAssigner_h
#define __DVAutoColourAssigner_h


#include <wx/wx.h>
#include <vector>

class wxDVAutoColourAssigner
{
public:
	wxDVAutoColourAssigner();
	virtual ~wxDVAutoColourAssigner();

	wxColour GetColourForIndex(int index);
	bool IsColourAssigned( int index );

	virtual wxColour AssignLineColour(int index);
	virtual void DeAssignLineColour(int index);

	void ResetColourList();
	void DeAssignAll();	
private:
	std::vector<wxColour> mAvailableColours;

	struct ColourPair
	{
		int index;
		wxColour colour;
	};

	std::vector<ColourPair> mAssignedColours;
};

#endif

