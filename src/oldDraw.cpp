void Group::onDraw(cairo_t* cr)
{
	double aBack = 0.0;

	if (mSHover || mSFocus)
		aBack = 0.5;
	if (mSHover && mSFocus)
		aBack = 0.8;

	int w = gtk_widget_get_allocated_width(GTK_WIDGET(mButton));
	int h = gtk_widget_get_allocated_height(GTK_WIDGET(mButton));

	if (aBack > 0)
	{
		// Drawing the main box that highlights the box
		cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, aBack);
		cairo_rectangle(cr, 0, 0, w, h);
		cairo_fill(cr);
	}

	// drawing the edge that gives the impression that there is more windows
	if (Settings::indicatorStyle == 0) // Bar
	{
		if (mSOpened)
		{
			// drawing status bar indicating a window is open
			if (mSFocus)
				cairo_set_source_rgba(cr, 0.30, 0.65, 0.90, 1);
			else
				cairo_set_source_rgba(cr, 0.7, 0.7, 0.7, 1);

#ifdef VERTICAL_BAR_ENABLED
			if (mDockPosition == DockPosition::Right ||
				(mDockPosition == DockPosition::Left && Settings::reverseIndicatorSide))
			{
				cairo_rectangle(cr, w * 0.9231, 0, w, h);
			}
			else if (mDockPosition == DockPosition::Left ||
				(mDockPosition == DockPosition::Right && Settings::reverseIndicatorSide))
			{
				cairo_rectangle(cr, 0, 0, w * 0.0769, h);
			}
			else if (mDockPosition == DockPosition::Top && !Settings::reverseIndicatorSide)
			{
				cairo_rectangle(cr, 0, 0, w, h * 0.0769);
			}
			else // if(mDockPosition == DockPosition::Bottom)
			{
				cairo_rectangle(cr, 0, h * 0.9231, w, h);
			}
#else
			cairo_rectangle(cr, 0, h * 0.9231, w, h);
#endif
			cairo_fill(cr);

#ifdef VERTICAL_BAR_ENABLED
			// handle having an extra blip if there are serveral windows in group
			if (mSMany && (mSOpened || mSHover))
			{
				if (mDockPosition == DockPosition::Right ||
					(mDockPosition == DockPosition::Left && Settings::reverseIndicatorSide))
				{
					cairo_rectangle(cr, w * 0.9231, 0, w, h * 0.12);
				}
				else if (mDockPosition == DockPosition::Left ||
					(mDockPosition == DockPosition::Right && Settings::reverseIndicatorSide))
				{
					cairo_rectangle(cr, 0, 0, w * 0.0679, h * 0.12);
				}
				cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.45);
				cairo_fill(cr);
			}
#endif
		}

		if (mSMany && (mSOpened || mSHover))
		{
#ifdef VERTICAL_BAR_ENABLED
			int x1, x2;
			cairo_pattern_t* pat;
			if ((mDockPosition == DockPosition::Right && !Settings::reverseIndicatorSide) ||
				(mDockPosition == DockPosition::Left && Settings::reverseIndicatorSide))
			{
				x1 = 0;
				x2 = (int)w * 0.12;
			}
			else
			{
				x1 = (int)w * 0.88;
				x2 = w;
			}
			pat = cairo_pattern_create_linear(x1, 0, x2, 0);
#else
			int x1 = (int)w * 0.88;
			cairo_pattern_t* pat = cairo_pattern_create_linear(x1, 0, w, 0);
#endif

			if (mDockPosition == DockPosition::Right)
			{
				cairo_pattern_add_color_stop_rgba(pat, 0.0, 0, 0, 0, 0.15);
				cairo_pattern_add_color_stop_rgba(pat, 0.1, 0, 0, 0, 0.35);
				cairo_pattern_add_color_stop_rgba(pat, 0.3, 0, 0, 0, 0.45);
			}
			else
			{
				cairo_pattern_add_color_stop_rgba(pat, 0.0, 0, 0, 0, 0.45);
				cairo_pattern_add_color_stop_rgba(pat, 0.1, 0, 0, 0, 0.35);
				cairo_pattern_add_color_stop_rgba(pat, 0.3, 0, 0, 0, 0.15);
			}

			if (aBack > 0) // if hovering or active
			{
#ifdef VERTICAL_BAR_ENABLED
				// if ((mDockPosition == DockPosition::Right && !Settings::reverseIndicatorSide) ||
				// (mDockPosition == DockPosition::Left && Settings::reverseIndicatorSide))
				// {
				cairo_rectangle(cr, x1, 0, x2, h);
				// }
				// else
				// {
				// cairo_rectangle(cr, x1, 0, x2, h);
				// }
#else
				cairo_rectangle(cr, x1, 0, w, h);
#endif
			}
			else
			{
#ifdef VERTICAL_BAR_ENABLED
				if (mDockPosition == DockPosition::Right || mDockPosition == DockPosition::Left)
				{
					// Do not do anything here this is taken care of before
					// cairo_rectangle(cr, x1, 0, x2, h);
				}
				else
				{
					cairo_rectangle(cr, x1, h * 0.9231, x2, h);
				}
#else
				cairo_rectangle(cr, x1, h * 0.9231, w, h);
#endif
			}
			cairo_pattern_add_color_stop_rgba(pat, 0.0, 0, 0, 0, 0.45);
			cairo_pattern_add_color_stop_rgba(pat, 0.1, 0, 0, 0, 0.35);
			cairo_pattern_add_color_stop_rgba(pat, 0.3, 0, 0, 0, 0.15);

			if (aBack > 0)
				cairo_rectangle(cr, x1, 0, w, h);
			else
				cairo_rectangle(cr, x1, round(h * 0.9231), w, h);

			cairo_set_source(cr, pat);
			cairo_fill(cr);

			cairo_pattern_destroy(pat);
		}
	}
	else if (Settings::indicatorStyle == 1) // Dots
	{
		if (mSOpened)
		{
			double dotRadius = std::max(h * (0.093), 2.);
#ifdef VERTICAL_BAR_ENABLED
			double epos;
			if ((mDockPosition == DockPosition::Left && !Settings::reverseIndicatorSide) ||
				(mDockPosition == DockPosition::Right && Settings::reverseIndicatorSide))
				epos = w * 0.01;
			else if ((mDockPosition == DockPosition::Right && !Settings::reverseIndicatorSide) ||
				(mDockPosition == DockPosition::Left && Settings::reverseIndicatorSide))
				epos = w * 0.99;
			else
				epos = h * 0.99;
#else
			double ypos = h * 0.99;
#endif

			double rgb[3] = {0, 1, 2};

			if (mSFocus)
			{
				rgb[0] = 0.30;
				rgb[1] = 0.65;
				rgb[2] = 0.90;
			}
			else
			{
				rgb[0] = 0.7;
				rgb[1] = 0.7;
				rgb[2] = 0.7;
			}

			if (mSMany)
			{
#ifdef VERTICAL_BAR_ENABLED
				double pos;
				if (mDockPosition == DockPosition::Right ||
					mDockPosition == DockPosition::Left)
				{
					pos = (h / 2.) - dotRadius * 1;
				}
				else
				{
					pos = (w / 2.) - dotRadius * 1;
				}
#else
				double cx = (w / 2.) - dotRadius * 1;
#endif

#ifdef VERTICAL_BAR_ENABLED
				double x, y;
				if (mDockPosition == DockPosition::Left ||
					mDockPosition == DockPosition::Right)
				{
					x = epos;
					y = pos;
				}
				else
				{
					x = pos;
					y = epos;
				}
				cairo_pattern_t* pat = cairo_pattern_create_radial(x, y, 0, x, y, dotRadius);
#else
				cairo_pattern_t* pat = cairo_pattern_create_radial(cx, ypos, 0, cx, ypos, dotRadius);
#endif

				cairo_pattern_add_color_stop_rgba(pat, 0.3, rgb[0], rgb[1], rgb[2], 1);
				cairo_pattern_add_color_stop_rgba(pat, 1, rgb[0], rgb[1], rgb[2], 0.15);
				cairo_set_source(cr, pat);

#ifdef VERTICAL_BAR_ENABLED
				cairo_arc(cr, x, y, dotRadius, 0.0, 2.0 * M_PI);
#else
				cairo_arc(cr, cx, ypos, dotRadius, 0.0, 2.0 * M_PI);
#endif
				cairo_fill(cr);

				cairo_pattern_destroy(pat);

#ifdef VERTICAL_BAR_ENABLED
				if (mDockPosition == DockPosition::Right ||
					mDockPosition == DockPosition::Left)
				{
					pos = (h / 2.) + dotRadius * 1;
				}
				else
				{
					pos = (w / 2.) + dotRadius * 1;
				}
#else
				cx = (w / 2.) + dotRadius * 1;
#endif

#ifdef VERTICAL_BAR_ENABLED
				if (mDockPosition == DockPosition::Left ||
					mDockPosition == DockPosition::Right)
				{
					x = epos;
					y = pos;
				}
				else
				{
					x = pos;
					y = epos;
				}
				pat = cairo_pattern_create_radial(x, y, 0, x, y, dotRadius);
#else
				pat = cairo_pattern_create_radial(cx, ypos, 0, cx, ypos, dotRadius);
#endif
				cairo_pattern_add_color_stop_rgba(pat, 0.3, rgb[0], rgb[1], rgb[2], 1);
				cairo_pattern_add_color_stop_rgba(pat, 1, rgb[0], rgb[1], rgb[2], 0.15);
				cairo_set_source(cr, pat);

#ifdef VERTICAL_BAR_ENABLED
				cairo_arc(cr, x, y, dotRadius, 0.0, 2.0 * M_PI);
#else
				cairo_arc(cr, cx, ypos, dotRadius, 0.0, 2.0 * M_PI);
#endif

				cairo_fill(cr);

				cairo_pattern_destroy(pat);
			}
			else
			{
#ifdef VERTICAL_BAR_ENABLED
				double pos;
				if (mDockPosition == DockPosition::Right ||
					mDockPosition == DockPosition::Left)
				{
					pos = h / 2.;
				}
				else
				{
					pos = w / 2.;
				}
#else
				double cx = w / 2.;
#endif

#ifdef VERTICAL_BAR_ENABLED
				double x, y;
				if (mDockPosition == DockPosition::Left ||
					mDockPosition == DockPosition::Right)
				{
					x = epos;
					y = pos;
				}
				else
				{
					x = pos;
					y = epos;
				}
				cairo_pattern_t* pat = cairo_pattern_create_radial(x, y, 0, x, y, dotRadius);
#else
				cairo_pattern_t* pat = cairo_pattern_create_radial(cx, ypos, 0, cx, ypos, dotRadius);
#endif
				cairo_pattern_add_color_stop_rgba(pat, 0.3, rgb[0], rgb[1], rgb[2], 1);
				cairo_pattern_add_color_stop_rgba(pat, 1, rgb[0], rgb[1], rgb[2], 0.15);
				cairo_set_source(cr, pat);

#ifdef VERTICAL_BAR_ENABLED
				cairo_arc(cr, x, y, dotRadius, 0.0, 2.0 * M_PI);
#else
				cairo_arc(cr, cx, ypos, dotRadius, 0.0, 2.0 * M_PI);
#endif
				cairo_fill(cr);

				cairo_pattern_destroy(pat);
			}
		}
	}
}

// jul 23 2020
void Group::onDraw(cairo_t* cr)
{
	double aBack = 0.0;

	if (mSHover || mSFocus)
		aBack = 0.5;
	if (mSHover && mSFocus)
		aBack = 0.8;

	int w = gtk_widget_get_allocated_width(GTK_WIDGET(mButton));
	int h = gtk_widget_get_allocated_height(GTK_WIDGET(mButton));

	if (aBack > 0)
	{
		// Drawing the main box that highlights the box
		cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, aBack);
		cairo_rectangle(cr, 0, 0, w, h);
		cairo_fill(cr);
	}

	// drawing the edge that gives the impression that there is more windows
	if (Settings::indicatorStyle == 0) // Bar
	{
		if (mSOpened)
		{
			// drawing status bar indicating a window is open
			if (mSFocus)
				cairo_set_source_rgba(cr, 0.30, 0.65, 0.90, 1);
			else
				cairo_set_source_rgba(cr, 0.7, 0.7, 0.7, 1);

#ifdef VERTICAL_BAR_ENABLED
			if (mDockPosition == DockPosition::Right ||
				(mDockPosition == DockPosition::Left && Settings::reverseIndicatorSide))
			{
				cairo_rectangle(cr, w * 0.9231, 0, w, h);
			}
			else if (mDockPosition == DockPosition::Left ||
				(mDockPosition == DockPosition::Right && Settings::reverseIndicatorSide))
			{
				cairo_rectangle(cr, 0, 0, w * 0.0769, h);
			}
			else if (mDockPosition == DockPosition::Top && !Settings::reverseIndicatorSide)
			{
				cairo_rectangle(cr, 0, 0, w, h * 0.0769);
			}
			else // if(mDockPosition == DockPosition::Bottom)
			{
				cairo_rectangle(cr, 0, h * 0.9231, w, h);
			}
#else
			cairo_rectangle(cr, 0, h * 0.9231, w, h);
#endif
			cairo_fill(cr);

#ifdef VERTICAL_BAR_ENABLED
			// handle having an extra blip if there are serveral windows in group
			// This is only for the left and right sides because they require to blips
			if (mSMany && (mSOpened || mSHover))
			{
				if (mDockPosition == DockPosition::Right ||
					(mDockPosition == DockPosition::Left && Settings::reverseIndicatorSide))
				{
					cairo_rectangle(cr, w * 0.9231, 0, w, h * 0.12);
				}
				else if (mDockPosition == DockPosition::Left ||
					(mDockPosition == DockPosition::Right && Settings::reverseIndicatorSide))
				{
					cairo_rectangle(cr, 0, 0, w * 0.0679, h * 0.12);
				}
				cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.45);
				cairo_fill(cr);
			}
#endif
		}

		if (mSMany && (mSOpened || mSHover))
		{
#ifdef VERTICAL_BAR_ENABLED
			int x1, x2;
			cairo_pattern_t* pat;
			if ((mDockPosition == DockPosition::Right && !Settings::reverseIndicatorSide) ||
				(mDockPosition == DockPosition::Left && Settings::reverseIndicatorSide))
			{
				x1 = 0;
				x2 = (int)w * 0.12;
			}
			else
			{
				x1 = (int)w * 0.88;
				x2 = w;
			}
			pat = cairo_pattern_create_linear(x1, 0, x2, 0);
#else
			int x1 = (int)w * 0.88;
			cairo_pattern_t* pat = cairo_pattern_create_linear(x1, 0, w, 0);
#endif

			if (mDockPosition == DockPosition::Right)
			{
				cairo_pattern_add_color_stop_rgba(pat, 0.0, 0, 0, 0, 0.15);
				cairo_pattern_add_color_stop_rgba(pat, 0.1, 0, 0, 0, 0.35);
				cairo_pattern_add_color_stop_rgba(pat, 0.3, 0, 0, 0, 0.45);
			}
			else
			{
				cairo_pattern_add_color_stop_rgba(pat, 0.0, 0, 0, 0, 0.45);
				cairo_pattern_add_color_stop_rgba(pat, 0.1, 0, 0, 0, 0.35);
				cairo_pattern_add_color_stop_rgba(pat, 0.3, 0, 0, 0, 0.15);
			}

			if (aBack > 0) // if hovering or active
			{
#ifdef VERTICAL_BAR_ENABLED
				// if ((mDockPosition == DockPosition::Right && !Settings::reverseIndicatorSide) ||
				// (mDockPosition == DockPosition::Left && Settings::reverseIndicatorSide))
				// {
				cairo_rectangle(cr, x1, 0, x2, h);
				// }
				// else
				// {
				// cairo_rectangle(cr, x1, 0, x2, h);
				// }
#else
				cairo_rectangle(cr, x1, 0, w, h);
#endif
			}
			else
			{
#ifdef VERTICAL_BAR_ENABLED
				if (mDockPosition == DockPosition::Right || mDockPosition == DockPosition::Left)
				{
					// Do not do anything here this is taken care of before
					// cairo_rectangle(cr, x1, 0, x2, h);
				}
				else
				{
					cairo_rectangle(cr, x1, h * 0.9231, x2, h);
				}
#else
				cairo_rectangle(cr, x1, h * 0.9231, w, h);
#endif
			}
			cairo_pattern_add_color_stop_rgba(pat, 0.0, 0, 0, 0, 0.45);
			cairo_pattern_add_color_stop_rgba(pat, 0.1, 0, 0, 0, 0.35);
			cairo_pattern_add_color_stop_rgba(pat, 0.3, 0, 0, 0, 0.15);

			if (aBack > 0)
				cairo_rectangle(cr, x1, 0, w, h);
			else
				cairo_rectangle(cr, x1, round(h * 0.9231), w, h);

			cairo_set_source(cr, pat);
			cairo_fill(cr);

			cairo_pattern_destroy(pat);
		}
	}
	else if (Settings::indicatorStyle == 1) // Dots
	{
		if (mSOpened)
		{
			double dotRadius = std::max(h * (0.093), 2.);
#ifdef VERTICAL_BAR_ENABLED
			double epos;
			if ((mDockPosition == DockPosition::Left && !Settings::reverseIndicatorSide) ||
				(mDockPosition == DockPosition::Right && Settings::reverseIndicatorSide))
				epos = w * 0.01;
			else if ((mDockPosition == DockPosition::Right && !Settings::reverseIndicatorSide) ||
				(mDockPosition == DockPosition::Left && Settings::reverseIndicatorSide))
				epos = w * 0.99;
			else
				epos = h * 0.99;
#else
			double ypos = h * 0.99;
#endif

			double rgb[3] = {0, 1, 2};

			if (mSFocus)
			{
				rgb[0] = 0.30;
				rgb[1] = 0.65;
				rgb[2] = 0.90;
			}
			else
			{
				rgb[0] = 0.7;
				rgb[1] = 0.7;
				rgb[2] = 0.7;
			}

			if (mSMany)
			{
#ifdef VERTICAL_BAR_ENABLED
				double pos;
				if (mDockPosition == DockPosition::Right ||
					mDockPosition == DockPosition::Left)
				{
					pos = (h / 2.) - dotRadius * 1;
				}
				else
				{
					pos = (w / 2.) - dotRadius * 1;
				}
#else
				double cx = (w / 2.) - dotRadius * 1;
#endif

#ifdef VERTICAL_BAR_ENABLED
				double x, y;
				if (mDockPosition == DockPosition::Left ||
					mDockPosition == DockPosition::Right)
				{
					x = epos;
					y = pos;
				}
				else
				{
					x = pos;
					y = epos;
				}
				cairo_pattern_t* pat = cairo_pattern_create_radial(x, y, 0, x, y, dotRadius);
#else
				cairo_pattern_t* pat = cairo_pattern_create_radial(cx, ypos, 0, cx, ypos, dotRadius);
#endif

				cairo_pattern_add_color_stop_rgba(pat, 0.3, rgb[0], rgb[1], rgb[2], 1);
				cairo_pattern_add_color_stop_rgba(pat, 1, rgb[0], rgb[1], rgb[2], 0.15);
				cairo_set_source(cr, pat);

#ifdef VERTICAL_BAR_ENABLED
				cairo_arc(cr, x, y, dotRadius, 0.0, 2.0 * M_PI);
#else
				cairo_arc(cr, cx, ypos, dotRadius, 0.0, 2.0 * M_PI);
#endif
				cairo_fill(cr);

				cairo_pattern_destroy(pat);

#ifdef VERTICAL_BAR_ENABLED
				if (mDockPosition == DockPosition::Right ||
					mDockPosition == DockPosition::Left)
				{
					pos = (h / 2.) + dotRadius * 1;
				}
				else
				{
					pos = (w / 2.) + dotRadius * 1;
				}
#else
				cx = (w / 2.) + dotRadius * 1;
#endif

#ifdef VERTICAL_BAR_ENABLED
				if (mDockPosition == DockPosition::Left ||
					mDockPosition == DockPosition::Right)
				{
					x = epos;
					y = pos;
				}
				else
				{
					x = pos;
					y = epos;
				}
				pat = cairo_pattern_create_radial(x, y, 0, x, y, dotRadius);
#else
				pat = cairo_pattern_create_radial(cx, ypos, 0, cx, ypos, dotRadius);
#endif
				cairo_pattern_add_color_stop_rgba(pat, 0.3, rgb[0], rgb[1], rgb[2], 1);
				cairo_pattern_add_color_stop_rgba(pat, 1, rgb[0], rgb[1], rgb[2], 0.15);
				cairo_set_source(cr, pat);

#ifdef VERTICAL_BAR_ENABLED
				cairo_arc(cr, x, y, dotRadius, 0.0, 2.0 * M_PI);
#else
				cairo_arc(cr, cx, ypos, dotRadius, 0.0, 2.0 * M_PI);
#endif

				cairo_fill(cr);

				cairo_pattern_destroy(pat);
			}
			else
			{
#ifdef VERTICAL_BAR_ENABLED
				double pos;
				if (mDockPosition == DockPosition::Right ||
					mDockPosition == DockPosition::Left)
				{
					pos = h / 2.;
				}
				else
				{
					pos = w / 2.;
				}
#else
				double cx = w / 2.;
#endif

#ifdef VERTICAL_BAR_ENABLED
				double x, y;
				if (mDockPosition == DockPosition::Left ||
					mDockPosition == DockPosition::Right)
				{
					x = epos;
					y = pos;
				}
				else
				{
					x = pos;
					y = epos;
				}
				cairo_pattern_t* pat = cairo_pattern_create_radial(x, y, 0, x, y, dotRadius);
#else
				cairo_pattern_t* pat = cairo_pattern_create_radial(cx, ypos, 0, cx, ypos, dotRadius);
#endif
				cairo_pattern_add_color_stop_rgba(pat, 0.3, rgb[0], rgb[1], rgb[2], 1);
				cairo_pattern_add_color_stop_rgba(pat, 1, rgb[0], rgb[1], rgb[2], 0.15);
				cairo_set_source(cr, pat);

#ifdef VERTICAL_BAR_ENABLED
				cairo_arc(cr, x, y, dotRadius, 0.0, 2.0 * M_PI);
#else
				cairo_arc(cr, cx, ypos, dotRadius, 0.0, 2.0 * M_PI);
#endif
				cairo_fill(cr);

				cairo_pattern_destroy(pat);
			}
		}
	}
}
