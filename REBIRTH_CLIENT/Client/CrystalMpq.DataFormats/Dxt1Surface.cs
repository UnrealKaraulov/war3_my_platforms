﻿#region Copyright Notice
// This file is part of CrystalMPQ.
// 
// Copyright (C) 2007-2011 Fabien BARBIER
// 
// CrystalMPQ is licenced under the Microsoft Reciprocal License.
// You should find the licence included with the source of the program,
// or at this URL: http://www.microsoft.com/opensource/licenses.mspx#Ms-RL
#endregion

using System;

namespace CrystalMpq.DataFormats
{
	public sealed class Dxt1Surface : DxtSurface
	{
		public Dxt1Surface(int width, int height, bool opaque = false, bool alphaPremultiplied = false)
			: base(width, height, opaque ? (byte)0 : (byte)1, alphaPremultiplied) { }

		public Dxt1Surface(byte[] rawData, int width, int height, bool opaque = false, bool alphaPremultiplied = false, bool shareBuffer = false)
			: base(rawData, width, height, opaque ? (byte)0 : (byte)1, alphaPremultiplied, shareBuffer) { }

		[CLSCompliant(false)]
		public unsafe Dxt1Surface(byte* rawData, int width, int height, bool opaque = false, bool alphaPremultiplied = false)
			: base(rawData, width, height, opaque ? (byte)0 : (byte)1, alphaPremultiplied) { }

		protected unsafe override void CopyToArgbInternal(SurfaceData surfaceData)
		{
			var colors = stackalloc ArgbColor[4];
			var nullColor = AlphaBitCount != 0 ? new ArgbColor(0, 0, 0, 0) : new ArgbColor(0, 0, 0);

			fixed (byte* dataPointer = data)
			{
				var destinationRowPointer = (byte*)surfaceData.DataPointer;
				var sourcePointer = dataPointer;
				int rowBlockStride = surfaceData.Stride << 2;

				for (int i = Height; i > 0; i -= 4, destinationRowPointer += rowBlockStride)
				{
					var destinationPointer = destinationRowPointer;

					for (int j = Width; j > 0; j -= 4)
					{
						ushort color0 = (ushort)(*sourcePointer++ | *sourcePointer++ << 8);
						ushort color1 = (ushort)(*sourcePointer++ | *sourcePointer++ << 8);

						colors[0] = new ArgbColor(color0);
						colors[1] = new ArgbColor(color1);

						if (color0 <= color1)
						{
							ArgbColor.DxtMergeHalves(colors + 2, colors, colors + 1);
							colors[3] = nullColor;
						}
						else
						{
							ArgbColor.DxtMergeThirds(colors + 2, colors + 1, colors);
							ArgbColor.DxtMergeThirds(colors + 3, colors, colors + 1);
						}

						// Handle the case where the surface's width is not a multiple of 4.
						int inverseBlockWidth = j > 4 ? 0 : 4 - j;

						var blockRowDestinationPointer = destinationPointer;

						for (int k = 4; k-- != 0; blockRowDestinationPointer += surfaceData.Stride)
						{
							byte rowData = *sourcePointer++;

							if (i + k < 4) continue; // Handle the case where the surface's height is not a multiple of 4.

							var blockDestinationPointer = (ArgbColor*)blockRowDestinationPointer;

							// The small loop here has been unrolled, which should be well worth it:
							//  - No loop variable is needed.
							//  - No useless shift and incrementation for the last step.
							//  - Only one conditional jump.
							//  - The loop will be executed very often, as the blocks are very small.
							switch (inverseBlockWidth)
							{
								case 0:
									*blockDestinationPointer++ = colors[rowData & 3];
									rowData >>= 2;
									goto case 1;
								case 1:
									*blockDestinationPointer++ = colors[rowData & 3];
									rowData >>= 2;
									goto case 2;
								case 2:
									*blockDestinationPointer++ = colors[rowData & 3];
									rowData >>= 2;
									goto case 3;
								case 3:
									*blockDestinationPointer = colors[rowData & 3];
									break;
							}
						}

						destinationPointer += 4 * sizeof(uint); // Skip the 4 processed pixels
					}
				}
			}
		}
	}
}
