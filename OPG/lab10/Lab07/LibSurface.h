#pragma once
#include <afxwin.h>
#include <cmath>
#include "CMatrix.h"
#include "LibGraph.h"

inline void DrawLightSphere(CDC& dc, double Radius, CMatrix& PView, CMatrix& PSourceLight, CRect RW, COLORREF Color, int Index)
{
	if (Radius <= 0.0 || RW.Width() <= 0 || RW.Height() <= 0)
		return;

	BYTE red = GetRValue(Color);
	BYTE green = GetGValue(Color);
	BYTE blue = GetBValue(Color);

	CMatrix VR = SphereToCart(PView);
	CMatrix VS = SphereToCart(PSourceLight);

	double modVR = ModVec(VR);
	if (modVR < 1e-7)
		return;

	double vx_dir = VR(0) / modVR;
	double vy_dir = VR(1) / modVR;
	double vz_dir = VR(2) / modVR;

	double camX = VR(0), camY = VR(1), camZ = VR(2);
	double srcX = VS(0), srcY = VS(1), srcZ = VS(2);

	CRectD RV(-Radius, Radius, Radius, -Radius);
	CMatrix MW = SpaceToWindow(RV, RW);
	CMatrix MV = CreateViewCoord(PView(0), PView(1), PView(2));

	double mv00 = MV(0, 0), mv01 = MV(0, 1), mv02 = MV(0, 2), mv03 = MV(0, 3);
	double mv10 = MV(1, 0), mv11 = MV(1, 1), mv12 = MV(1, 2), mv13 = MV(1, 3);
	double mw00 = MW(0, 0), mw02 = MW(0, 2);
	double mw11 = MW(1, 1), mw12 = MW(1, 2);

	double rPx = 0.5 * (RW.Width() > RW.Height() ? RW.Width() : RW.Height());
	double dq = (0.6 / rPx) * (180.0 / pi);
	if (dq > 0.5) dq = 0.5;
	if (dq < 0.08) dq = 0.08;
	const double p = 10.0;

	for (double q = 0.0; q <= 180.0; q += dq)
	{
		double q_rad = (q / 180.0) * pi;
		double sin_q = sin(q_rad);
		double cos_q = cos(q_rad);

		double df = (fabs(sin_q) > 1e-3) ? (dq / fabs(sin_q)) : 360.0;

		for (double fi = 0.0; fi < 360.0; fi += df)
		{
			double fi_rad = (fi / 180.0) * pi;
			double cos_fi = cos(fi_rad);
			double sin_fi = sin(fi_rad);

			double nx = sin_q * cos_fi;
			double ny = sin_q * sin_fi;
			double nz = cos_q;

			double cos_RN = nx * vx_dir + ny * vy_dir + nz * vz_dir;
			if (cos_RN >= 0.0)
			{
				double px = Radius * nx;
				double py = Radius * ny;
				double pz = Radius * nz;

				double xv = mv00 * px + mv01 * py + mv02 * pz + mv03;
				double yv = mv10 * px + mv11 * py + mv12 * pz + mv13;

				int xw = static_cast<int>(floor(mw00 * xv + mw02 + 0.5));
				int yw = static_cast<int>(floor(mw11 * yv + mw12 + 0.5));

				double lx = srcX - px;
				double ly = srcY - py;
				double lz = srcZ - pz;
				double modL = sqrt(lx * lx + ly * ly + lz * lz);

				double kLight = 0.0;
				if (modL > 1e-7)
				{
					lx /= modL;
					ly /= modL;
					lz /= modL;

					double cos_LN = lx * nx + ly * ny + lz * nz;
					if (cos_LN > 0.0)
					{
						if (Index == 0)
						{
							kLight = cos_LN;
						}
						else if (Index == 1)
						{
							double rx = 2.0 * cos_LN * nx - lx;
							double ry = 2.0 * cos_LN * ny - ly;
							double rz = 2.0 * cos_LN * nz - lz;

							double ex = camX - px;
							double ey = camY - py;
							double ez = camZ - pz;
							double modE = sqrt(ex * ex + ey * ey + ez * ez);

							if (modE > 1e-7)
							{
								double cos_RE = (rx * ex + ry * ey + rz * ez) / modE;
								if (cos_RE > 0.0)
									kLight = pow(cos_RE, p);
							}
						}
					}
				}

				if (kLight < 0.0) kLight = 0.0;
				if (kLight > 1.0) kLight = 1.0;

				COLORREF Col = RGB(
					static_cast<BYTE>(kLight * red),
					static_cast<BYTE>(kLight * green),
					static_cast<BYTE>(kLight * blue)
				);
				dc.SetPixelV(xw, yw, Col);
			}
		}
	}
}