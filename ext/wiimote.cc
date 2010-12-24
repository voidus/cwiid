/* ruby-cwiid - Wii Remote interface.
   Copyright (C) 2010 Jan Wedekind

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */
#include "wiimote.hh"

using namespace std;

VALUE WiiMote::cRubyClass = Qnil;

WiiMote *WiiMote::current = NULL;

static void staticErr( cwiid_wiimote_t *wiimote, const char *s, va_list ap )
{
  WiiMote::current->err( s, ap );
}

WiiMote::WiiMote(void) throw (Error):
  m_wiimote(NULL), m_error(false)
{
  bdaddr_t bdaddrAny = { { 0, 0, 0, 0, 0, 0 } };
  current = this;
  cwiid_set_err( staticErr );
  m_wiimote = cwiid_open( &bdaddrAny, 0 );
  ERRORMACRO( !m_error, Error, , "Failed to connect to Wii Remote: "
              << m_errorMsg );
}

WiiMote::~WiiMote(void)
{
  close();
}

void WiiMote::close(void)
{
  if ( m_wiimote != NULL ) {
    cwiid_close( m_wiimote );
    m_wiimote = NULL;
  };
}

void WiiMote::err( const char *s, va_list ap )
{
  char buffer[4096];
  vsnprintf( &buffer[0], sizeof(buffer), s, ap );
  m_errorMsg = buffer;
  m_error = true;
}

VALUE WiiMote::registerRubyClass(void)
{
  cRubyClass = rb_define_class( "WiiMote", rb_cObject );
  rb_define_singleton_method( cRubyClass, "new",
                              RUBY_METHOD_FUNC( wrapNew ), 0 );
  rb_define_method( cRubyClass, "close", RUBY_METHOD_FUNC( wrapClose ), 0 );
  return cRubyClass;
}

void WiiMote::deleteRubyObject( void *ptr )
{
  delete (WiiMotePtr *)ptr;
}

VALUE WiiMote::wrapNew( VALUE rbClass )
{
  VALUE retVal = Qnil;
  try {
    WiiMotePtr ptr( new WiiMote );
    retVal = Data_Wrap_Struct( rbClass, 0, deleteRubyObject,
                               new WiiMotePtr( ptr ) );
  } catch ( exception &e ) {
    rb_raise( rb_eRuntimeError, "%s", e.what() );
  };
  return retVal;
}

VALUE WiiMote::wrapClose( VALUE rbSelf )
{
  WiiMotePtr *self; Data_Get_Struct( rbSelf, WiiMotePtr, self );
  (*self)->close();
  return rbSelf;
}
 