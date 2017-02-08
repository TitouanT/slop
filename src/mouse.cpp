#include "mouse.hpp"

void Mouse::setButton( int button, int state ) {
    for (unsigned int i=0;i<buttons.size();i++ ) {
        if ( buttons[i].x == button ) {
            buttons[i].y = state;
            return;
        }
    }
    buttons.push_back(glm::ivec2(button,state));
}

int Mouse::getButton( int button ) {
    for (unsigned int i=0;i<buttons.size();i++ ) {
        if ( buttons[i].x == button ) {
            return buttons[i].y;
        }
    }
    return 0;
}

glm::vec2 Mouse::getMousePos() {
    Window root, child;
    int mx, my;
    int wx, wy;
    unsigned int mask;
    XQueryPointer( x11->display, x11->root, &root, &child, &mx, &my, &wx, &wy, &mask );
    return glm::vec2( mx, my );
}

void Mouse::setCursor( int cursor ) {
    if ( currentCursor == cursor ) {
        return;
    }
    XFreeCursor( x11->display, xcursor );
    xcursor = XCreateFontCursor( x11->display, cursor );
    XChangeActivePointerGrab( x11->display,
                              PointerMotionMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask,
                              xcursor, CurrentTime );
}

Mouse::Mouse(X11* x11, bool nodecorations ) {
    this->x11 = x11;
    currentCursor = XC_cross;
    xcursor = XCreateFontCursor( x11->display, XC_cross );
    hoverWindow = None;
    XGrabPointer( x11->display, x11->root, True,
                  PointerMotionMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask,
                  GrabModeAsync, GrabModeAsync, None, xcursor, CurrentTime );

    Window root;
    int mx, my;
    int wx, wy;
    unsigned int mask;
    if ( nodecorations ) {
        // Get the deepest available window if we don't want decorations.
        Window child = x11->root;
        while( child ) {
            hoverWindow = child;
            XQueryPointer( x11->display, child, &root, &child, &mx, &my, &wx, &wy, &mask );
        }
    } else {
        XQueryPointer( x11->display, x11->root, &root, &hoverWindow, &mx, &my, &wx, &wy, &mask );
    }
    selectAllInputs( x11->root, nodecorations );
}

Mouse::~Mouse() {
	XUngrabPointer( x11->display, CurrentTime );
}

void Mouse::update() {
    XEvent event;
    while ( XCheckTypedEvent( x11->display, ButtonPress, &event ) ) {
		setButton( event.xbutton.button, 1 );
	}
    while ( XCheckTypedEvent( x11->display, ButtonRelease, &event ) ) {
		setButton( event.xbutton.button, 0 );
	}
    while ( XCheckTypedEvent( x11->display, EnterNotify, &event ) ) {
        hoverWindow = event.xcrossing.window;
	}
}

// This cheesy function makes sure we get all EnterNotify events on ALL the windows.
void Mouse::selectAllInputs( Window win, bool nodecorations ) {
    Window root, parent;
    Window* children;
    unsigned int nchildren;
    XQueryTree( x11->display, win, &root, &parent, &children, &nchildren );
    for ( unsigned int i=0;i<nchildren;i++ ) {
            XSelectInput( x11->display, children[ i ], EnterWindowMask );
        if ( nodecorations ) {
            selectAllInputs( children[i], nodecorations );
        }
    }
    free( children );
}
