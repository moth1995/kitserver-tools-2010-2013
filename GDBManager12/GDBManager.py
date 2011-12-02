# GDB Manager
# Version 12.0.1
# by Juce.

import wx
import wx.lib.scrolledpanel as scrolled
import wx.lib.colourselect as csel
import string, math, re
import sys, os, cStringIO, codecs
import yaml

VERSION, DATE = "12.0.1", "12/2012"
DEFAULT_PNG = os.getcwd() + "/default.png"
CONFIG_FILE = os.getcwd() + "/gdbm.yaml"
WINDOW_TITLE = "GDB Manager 12"
FRAME_WIDTH = 800
FRAME_HEIGHT = 600

DEFAULT_MASK = "mask.png"
MASK_COLORS = {
    "shirt":0x0000ff,
    "shorts":0xff0000,
    "socks":0x00ffff,
}

techfitModels = [34,35,36,37,38,44,55,56,57,65,69,100]
maxModel = 1000

overlayPositions = {
    "pes2012":{
        "numbers":[232,46], 
        "chest-1":[91,46], 
        "shorts-left":[130,195],
        "shorts-right":[47,195],
        "nameA-type1":[228,21],
        "nameB-type1":[240,21],
        "nameC-type1":[252,21],
        "nameA-type2":[225,21],
        "nameB-type2":[240,21],
        "nameC-type2":[250,21],
        "nameA-type3":[223,21],
        "nameB-type3":[240,21],
        "nameC-type3":[250,21],
        "nameA-type4":[221,21],
        "nameB-type4":[240,21],
        "nameC-type4":[250,21],
        "num-width":32,
        "num-height":64,
        "shorts-num-width":16,
        "shorts-num-height":32,
        "chest-num-width":16,
        "chest-num-height":32,
        "font-width":14,
        "font-height":18,
        "front-number-y-mult":2.4,
        "front-number-x-mult":2.8,
        "front-number-size-mult":1.0,
        "name-y-mult":3.0,
        "shorts-number-y-mult":2.4,
        "shorts-number-x-mult":2.4,
        "shorts-number-size-mult":1.0,
    },
    "pes2012-techfit":{
        "numbers":[232-37,42], 
        "chest-1":[91-15,42], 
        "shorts-left":[125+250,205],
        "shorts-right":[55+260,205],
        "nameA-type1":[228-37,14],
        "nameB-type1":[240-37,14],
        "nameC-type1":[252-37,14],
        "nameA-type2":[225-37,14],
        "nameB-type2":[240-37,14],
        "nameC-type2":[250-37,14],
        "nameA-type3":[223-37,14],
        "nameB-type3":[240-37,14],
        "nameC-type3":[250-37,14],
        "nameA-type4":[221-37,14],
        "nameB-type4":[240-37,14],
        "nameC-type4":[250-37,14],
        "num-width":32,
        "num-height":64,
        "shorts-num-width":16,
        "shorts-num-height":32,
        "chest-num-width":16,
        "chest-num-height":32,
        "font-width":14,
        "font-height":18,
        "front-number-y-mult":2.4,
        "front-number-x-mult":2.3,
        "front-number-size-mult":0.7,
        "name-y-mult":2.3,
        "shorts-number-y-mult":1.8,
        "shorts-number-x-mult":2.2,
        "shorts-number-size-mult":0.5,
    },
}

def getKitTemplateType(model):
    if int(model) in techfitModels:
        return "pes2012-techfit"
    return "pes2012"

def getCommonDir(path,base):
    npath = os.path.normcase(path).split('\\')
    nbase = os.path.normcase(base).split('\\')
    common = [s for (s,i) in zip(npath,range(min(len(npath),len(nbase)))) if nbase[i]==s]
    return '\\'.join(common)

def makeRelativePath(path, base):
    commonDir = getCommonDir(path,base)
    if len(commonDir)==0:
        return path
    relPath = path[len(commonDir)+1:]
    baseRest = base[len(commonDir)+1:]
    if len(baseRest)>0:
        relPath = "%s%s" % ("../" * len(baseRest.split("\\")), relPath)
    return os.path.normcase(relPath)

def isNationalKit(teamId):
    return teamId < 86 or teamId == 274

"""
Read kit attributes from config.txt file
"""
def readAttributes(kit):
    # don't read again, if already done so
    if kit.attribRead:
        return

    # clear out the attributes dictionary
    kit.attributes.clear()

    #print "Reading attributes for %s" % kit.foldername
    att, section = None, ""
    try:
        att = codecs.open("%s/%s" % (kit.foldername, "config.txt"), "rt", "utf-8")
        found = False
        for line in att:
            line = line.strip()
            #print "line: {%s}" % line

            # strip out the comment
            commentStarts = line.find("#")
            if commentStarts != -1:
                line = line[:commentStarts]

            if len(line.strip())==0:
                continue

            # work around BOM-bug
            if line[0]==u'\ufeff':
                line = line[1:]

            tok = line.split('=',2)
            if len(tok)==2:
                val = tok[1].strip()
                if val[0]=='"' and val[-1]=='"': val = val[1:-1]
                kit.attributes[tok[0].strip()] = val

        att.close()

    except IOError:
        # unable to read attributes. Ignore.
        if att != None:
            att.close()

    # drop 'shorts.number.show' attribute
    try: del kit.attributes['shorts.number.show']
    except KeyError: pass

    #print kit.attributes
    kit.attribRead = True


class RGBAColor:
    def __init__(self, color, alpha=-1):
        self.color = color
        self.alpha = alpha


"""
Utility method to construct wx.Color object 
from a RRGGBBAA string, as used in config.txt files
"""
def MakeRGBAColor(str):
    r, g, b = int(str[0:2],16), int(str[2:4],16), int(str[4:6],16)
    try:
        a = int(str[6:8], 16)
        return RGBAColor(wx.Color(r,g,b), a)
    except:
        return RGBAColor(wx.Color(r,g,b), -1)


"""
Utility method for showing message box window
"""
def MessageBox(owner, title, text):
    dlg = wx.MessageDialog(owner, text, title, wx.OK | wx.ICON_INFORMATION)
    dlg.ShowModal()
    dlg.Destroy()

"""
A panel with colour select button, label, and edit control
"""
class KitColourSelect(wx.Panel):
    def __init__(self, parent, attribute, labelText, frame):
        wx.Panel.__init__(self, parent, -1)

        self.undef = wx.Color(0x99,0x99,0x99)
        self.att = attribute
        #self.label = wx.StaticText(self, -1, labelText, size=(120, -1), style=wx.ALIGN_RIGHT)
        #font = wx.Font(12, wx.SWISS, wx.NORMAL, wx.NORMAL)
        #self.label.SetFont(font)
        #self.label.SetSize(self.label.GetBestSize())

        ##########################
        self.label = wx.StaticText(self, -1, labelText, size=(180,-1), style=wx.ALIGN_RIGHT)
        self.label.SetBackgroundColour(wx.Colour(230,230,230))
        font = wx.Font(10, wx.SWISS, wx.NORMAL, wx.NORMAL)
        self.label.SetFont(font)
        self.label.SetSize(self.label.GetBestSize())

        #self.choice = wx.Choice(self, -1, choices=[str(i) for i in items], size=(100,-1))
        #self.choice.SetSelection(0)
        #self.button = wx.Button(self, -1, "undef", size=(60,1))

        #self.sizer = wx.BoxSizer(wx.HORIZONTAL)
        #self.sizer.Add(self.button, 0, wx.RIGHT | wx.EXPAND, border=10)
        #self.sizer.Add(self.label, 0, wx.RIGHT | wx.ALIGN_CENTER_VERTICAL, border=10)
        #self.sizer.Add(self.choice, 0, wx.EXPAND)
        ##############################


        self.cs = csel.ColourSelect(self, -1, "", self.undef, size=(40,-1))
        self.edit = wx.TextCtrl(self, -1, "undefined", style=wx.TE_PROCESS_ENTER, validator=MyValidator(), size=(80,-1))
        self.edit.SetMaxLength(8)
        self.button = wx.Button(self, -1, "undef", size=(60, -1)) 
        self.frame = frame

        csSizer = wx.BoxSizer(wx.HORIZONTAL)
        csSizer.Add(self.cs, 0, wx.EXPAND)
        csSizer.Add(self.edit, 0, wx.EXPAND)

        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.Add(wx.Panel(self,-1), 1, wx.EXPAND)
        sizer.Add(self.button, 0, wx.RIGHT | wx.ALIGN_CENTER_VERTICAL, border=10)
        sizer.Add(self.label, 0, wx.EXPAND)
        sizer.Add(csSizer, 0, wx.LEFT | wx.EXPAND, border=10)
        sizer.Add(wx.Panel(self,-1), 1, wx.EXPAND)

        self.SetSizer(sizer)
        self.Layout()

        self.cs.Bind(csel.EVT_COLOURSELECT, self.OnSelectColour)
        self.edit.Bind(wx.EVT_TEXT_ENTER, self.OnEditColour)
        self.button.Bind(wx.EVT_BUTTON, self.OnUndef)

        self.frame.bindKeyEvents(
                [self, self.edit, self.button, self.cs, self.label])


    def SetColour(self, color):
        self.cs.SetColour(color)
        self.edit.SetValue("%02X%02X%02X" % (color.Red(), color.Green(), color.Blue()))
        # update the kit panel
        try:
            self.frame.kitPanel.kit.attributes[self.att] = self.edit.GetValue()
            #self.frame.kitPanel.Refresh()
        except AttributeError:
            pass
        except KeyError:
            pass


    def ModifyColour(self, color):
        self.SetColour(color)

        # add to modified list
        self.frame.addKitToModified()


    def SetRGBAColour(self, rgba):
        color = rgba.color
        self.cs.SetColour(color)
        if rgba.alpha == -1:
            self.edit.SetValue("%02X%02X%02X" % (color.Red(), color.Green(), color.Blue()))
        else:
            self.edit.SetValue("%02X%02X%02X%02X" % (color.Red(), color.Green(), color.Blue(), rgba.alpha))
        # update the kit panel
        try:
            self.frame.kitPanel.kit.attributes[self.att] = self.edit.GetValue()
            #self.frame.kitPanel.Refresh()
        except AttributeError:
            pass
        except KeyError:
            pass


    def ClearColour(self):
        self.cs.SetColour(self.undef)
        self.edit.SetValue("undefined")
        # update the kit panel
        try:
            del self.frame.kitPanel.kit.attributes[self.att]
            #self.frame.kitPanel.Refresh()
        except AttributeError:
            pass
        except KeyError:
            pass


    """
    Sets attribute to newly selected color
    """
    def OnSelectColour(self, event):
        self.SetColour(event.GetValue())

        # add to modified list
        self.frame.addKitToModified()


    """
    Verifies manually edited color and sets attribute
    """
    def OnEditColour(self, event):
        text = self.edit.GetValue()
        # add padding zeroes, if needed
        if len(text) < 6:
            text = "%s%s" % ('0'*(6-len(text)), text)

        # attempt to set the color
        color = self.undef
        try:
            color = MakeRGBAColor(text)
            self.SetRGBAColour(color)
        except:
            self.ClearColour()

        # add to modified list
        self.frame.addKitToModified()


    """
    Removes color definition from attributes
    """
    def OnUndef(self, event):
        self.ClearColour()

        # add to modified list
        self.frame.addKitToModified()


class MyValidator(wx.PyValidator):
    def __init__(self):
        wx.PyValidator.__init__(self)
        self.Bind(wx.EVT_CHAR, self.OnChar)

    def Clone(self):
        return MyValidator()

    def Validate(self, win):
        tc = self.GetWindow()
        val = tc.GetValue()

        for x in val:
            if x not in string.hexdigits:
                return False

        return True

    def OnChar(self, event):
        key = event.GetKeyCode()

        if key < wx.WXK_SPACE or key == wx.WXK_DELETE or key > 255:
            event.Skip()
            return

        if chr(key) in string.hexdigits:
            event.Skip()
            return

        if not wx.Validator_IsSilent():
            wx.Bell()

        # Returning without calling even.Skip eats the event before it
        # gets to the text control
        return

"""
A drop-down list with label
"""
class MyKeyList(wx.Panel):
    def __init__(self, parent, labelText, items, frame):
        wx.Panel.__init__(self, parent, -1)
        self.frame = frame
        self.label = wx.StaticText(self, -1, labelText, size=(160,-1), style=wx.ALIGN_RIGHT)
        self.label.SetBackgroundColour(wx.Colour(230,230,230))
        font = wx.Font(10, wx.SWISS, wx.NORMAL, wx.NORMAL)
        self.label.SetFont(font)
        self.label.SetSize(self.label.GetBestSize())

        self.choice = wx.Choice(self, -1, choices=[str(i) for i in items], size=(140,-1))
        self.choice.SetSelection(0)

        self.sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.sizer.Add(self.label, 0, wx.RIGHT | wx.ALIGN_CENTER_VERTICAL, border=10)
        self.sizer.Add(self.choice, 0, wx.EXPAND)

        # by default the kit panel is not refreshed on selection change
        self.refreshOnChange = False

        # bind events
        self.choice.Bind(wx.EVT_CHOICE, self.OnSelect)

        self.frame.bindKeyEvents(
                [self, self.choice, self.label])

        self.SetSizer(self.sizer)
        self.Layout()


    def OnSelect(self, event):
        selection = event.GetString()
        print "Shorts selected: %s" % selection
        self.frame.kitPanel.kit.shortsKey = selection
        # bind kit to shorts-kit
        kit = self.choice.GetClientData(self.choice.GetSelection())
        self.frame.shortsNumLocation.kit = kit
        self.frame.shortsNumLocation.SetStringSelection()
        self.frame.numpal.SetStringSelection()
        self.frame.kitPanel.Refresh()


"""
A drop-down list with label
"""
class MyList(wx.Panel):
    def __init__(self, parent, attribute, labelText, items, frame, deprecatedMap={}, kit=None):
        wx.Panel.__init__(self, parent, -1)
        self.frame = frame
        self.kit = kit
        self.att = attribute
        self.items = items
        self.deprecatedMap = deprecatedMap
        self.label = wx.StaticText(self, -1, labelText, size=(200,-1), style=wx.ALIGN_RIGHT)
        self.label.SetBackgroundColour(wx.Colour(230,230,230))
        font = wx.Font(10, wx.SWISS, wx.NORMAL, wx.NORMAL)
        self.label.SetFont(font)
        self.label.SetSize(self.label.GetBestSize())

        #self.choice = wx.ComboBox(self, -1, choices=[str(i) for i in items], size=(100,-1), style=wx.CB_DROPDOWN | wx.TE_PROCESS_ENTER)
        self.choice = wx.Choice(self, -1, choices=[str(i) for i in items], size=(100,-1))
        self.choice.SetSelection(0)
        self.button = wx.Button(self, -1, "undef", size=(60,1))

        self.sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.sizer.Add(wx.Panel(self, -1), 1, wx.EXPAND)
        self.sizer.Add(self.button, 0, wx.RIGHT | wx.EXPAND, border=10)
        self.sizer.Add(self.label, 0, wx.RIGHT | wx.ALIGN_CENTER_VERTICAL, border=10)
        self.sizer.Add(self.choice, 0)
        self.sizer.Add(wx.Panel(self, -1), 1, wx.EXPAND)

        # by default the kit panel is not refreshed on selection change
        self.refreshOnChange = False

        # bind events
        self.choice.Bind(wx.EVT_CHOICE, self.OnSelect)
        #self.choice.Bind(wx.EVT_COMBOBOX, self.OnSelect)
        #self.choice.Bind(wx.EVT_TEXT_ENTER, self.OnSelect)
        self.button.Bind(wx.EVT_BUTTON, self.OnUndef)

        self.frame.bindKeyEvents(
                [self, self.choice, self.button, self.label])

        self.SetSizer(self.sizer)
        self.Layout()


    def getKit(self):
        kit = self.kit
        if kit == None: kit = self.frame.kitPanel.kit
        return kit


    def SetStringSelection(self, s=None):
        kit = self.getKit()
        if s == None:
            try: s = kit.attributes[self.att]
            except KeyError: s = "undefined"
        # check for deprecated values
        if not s in self.items:
            s = self.deprecatedMap.get(s,s)
        #print "{%s}" % s
        if not s in [str(x) for x in self.items]:
            self.items.append(s)
            self.choice.Append(s)
        self.choice.SetStringSelection(s)
        kit.attributes[self.att] = s
        if self.refreshOnChange:
            self.frame.kitPanel.Refresh()


    def SetUndef(self):
        kit = self.getKit()
        self.choice.SetSelection(0)
        try:
            del kit.attributes[self.att] 
        except AttributeError:
            pass
        except KeyError:
            pass
        if self.refreshOnChange:
            self.frame.kitPanel.Refresh()


    def OnSelect(self, event):
        kit = self.getKit()
        selection = event.GetString()
        index = self.choice.GetSelection()
        if index == 0:
            # first item should always be "undefined"
            self.SetUndef()
        else:
            self.SetStringSelection(selection)

        # add kit to modified set
        self.frame.addKitToModified(kit)


    def OnUndef(self, event):
        kit = self.getKit()
        self.SetUndef()

        # add kit to modified set
        self.frame.addKitToModified(kit)

"""
A text field choice with label
"""
class MyTextField(wx.Panel):
    def __init__(self, parent, attribute, labelText, value, rootPath, frame, kit=None):
        wx.Panel.__init__(self, parent, -1)
        self.rootPath = rootPath
        self.frame = frame
        self.kit = kit
        self.att = attribute
        self.label = wx.StaticText(self, -1, labelText, size=(100,-1), style=wx.ALIGN_RIGHT)
        self.label.SetBackgroundColour(wx.Colour(230,230,230))
        font = wx.Font(10, wx.SWISS, wx.NORMAL, wx.NORMAL)
        self.label.SetFont(font)
        self.label.SetSize(self.label.GetBestSize())

        self.text = wx.TextCtrl(self, -1, "", size=(200,-1))
        self.button = wx.Button(self, -1, "undef", size=(60,1))

        self.sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.sizer.Add(wx.Panel(self,-1), 1, wx.EXPAND)
        self.sizer.Add(self.button, 0, wx.RIGHT | wx.EXPAND, border=10)
        self.sizer.Add(self.label, 0, wx.RIGHT | wx.ALIGN_CENTER_VERTICAL, border=10)
        self.sizer.Add(self.text, 0, wx.EXPAND)
        self.sizer.Add(wx.Panel(self,-1), 1, wx.EXPAND)

        # by default the kit panel is not refreshed on selection change
        self.refreshOnChange = False

        # bind events
        #self.text.Bind(wx.EVT_CHOICE, self.OnSelect)
        self.button.Bind(wx.EVT_BUTTON, self.OnUndef)
        self.text.Bind(wx.EVT_TEXT, self.OnTextChange)

        self.frame.bindKeyEvents(
                [self, self.text, self.button, self.label])

        self.SetSizer(self.sizer)
        self.Layout()


    def getKit(self):
        kit = self.kit
        if kit == None: kit = self.frame.kitPanel.kit
        return kit


    def SetStringSelection(self, str):
        kit = self.getKit()
        self.text.SetValue(str)
        kit.attributes[self.att] = str
        if self.refreshOnChange:
            self.frame.kitPanel.Refresh()

    def SetUndef(self):
        kit = self.getKit()
        self.text.SetValue("")
        try:
            del kit.attributes[self.att] 
        except AttributeError:
            pass
        except KeyError:
            pass
        if self.refreshOnChange:
            self.frame.kitPanel.Refresh()

    def OnUndef(self, event):
        kit = self.getKit()
        self.SetUndef()

        # add kit to modified set
        self.frame.addKitToModified(kit)

    def OnTextChange(self, event):
        kit = self.getKit()
        if kit != None:
            oldVal = kit.attributes.get(self.att,"")
            newVal = self.text.GetValue()
            if newVal != oldVal:
                #print "Description modified: old={%s}, new={%s}" % (oldVal,newVal)
                kit.attributes[self.att] = newVal
                # add kit to modified set
                self.frame.addKitToModified(kit)


"""
A file choice with label
"""
class MyNumbersFile(wx.Panel):
    def __init__(self, parent, attribute, labelText, value, rootPath, frame, kit=None):
        wx.Panel.__init__(self, parent, -1)
        self.rootPath = rootPath
        self.frame = frame
        self.kit = kit
        self.att = attribute
        self.label = wx.StaticText(self, -1, labelText, size=(100,-1), style=wx.ALIGN_RIGHT)
        self.label.SetBackgroundColour(wx.Colour(230,230,230))
        font = wx.Font(10, wx.SWISS, wx.NORMAL, wx.NORMAL)
        self.label.SetFont(font)
        self.label.SetSize(self.label.GetBestSize())

        self.text = wx.TextCtrl(self, -1, "", size=(170,-1))
        self.text.SetEditable(False)
        self.button = wx.Button(self, -1, "undef", size=(60,1))
        self.fileButton = wx.Button(self, -1, "...", size=(30,1))

        self.sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.sizer.Add(self.button, 0, wx.RIGHT | wx.EXPAND, border=10)
        self.sizer.Add(self.label, 0, wx.RIGHT | wx.ALIGN_CENTER_VERTICAL, border=10)
        self.sizer.Add(self.text, 0, wx.EXPAND)
        self.sizer.Add(self.fileButton, 0, wx.EXPAND)

        # by default the kit panel is not refreshed on selection change
        self.refreshOnChange = False

        # bind events
        #self.text.Bind(wx.EVT_CHOICE, self.OnSelect)
        self.button.Bind(wx.EVT_BUTTON, self.OnUndef)
        self.fileButton.Bind(wx.EVT_BUTTON, self.OnChooseFile)

        self.SetSizer(self.sizer)
        self.Layout()


    def getKit(self):
        kit = self.kit
        if kit == None: kit = self.frame.kitPanel.kit
        return kit


    def SetStringSelection(self, str):
        kit = self.getKit()
        self.text.SetValue(str)
        kit.attributes[self.att] = str
        if self.refreshOnChange:
            self.frame.kitPanel.Refresh()

    def SetUndef(self):
        kit = self.getKit()
        self.text.SetValue("")
        try:
            del kit.attributes[self.att] 
        except AttributeError:
            pass
        except KeyError:
            pass
        if self.refreshOnChange:
            self.frame.kitPanel.Refresh()

    def OnUndef(self, event):
        kit = self.getKit()
        self.SetUndef()

        # add kit to modified set
        self.frame.addKitToModified(kit)

    def OnChooseFile(self, event):
        wildcard = "PNG images (*.png)|*.png|" \
                   "BMP images (*.bmp)|*.bmp"

        kit = self.getKit()
        currdir = kit.foldername

        dlg = wx.FileDialog(
            self, message="Choose a file", defaultDir=currdir, 
            defaultFile="", wildcard=wildcard, style=wx.OPEN | wx.CHANGE_DIR
            )

        # Show the dialog and retrieve the user response. If it is the OK response, 
        # process the data.
        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            files = dlg.GetPaths()
            if len(files)>0:
                newfile = os.path.normcase(files[0])
                # make relative path
                self.SetStringSelection(makeRelativePath(newfile, kit.foldername))

                # add kit to modified set
                self.frame.addKitToModified(kit)


"""
A file choice with label
"""
class MyShortsNumPalFile(MyNumbersFile):

    def SetStringSelection(self, str=None):
        kit = self.getKit()
        if str == None:
            try: str = kit.attributes[self.att % kit.shortsKey]
            except KeyError: str = "undefined"
        self.text.SetValue(str)
        kit.attributes[self.att % kit.shortsKey] = str
        if self.refreshOnChange:
            self.frame.kitPanel.Refresh()

    def SetUndef(self):
        kit = self.getKit()
        self.text.SetValue("")
        try:
            del kit.attributes[self.att % kit.shortsKey] 
            del kit.attributes["shorts.num-pal"]
        except AttributeError:
            pass
        except KeyError:
            pass
        if self.refreshOnChange:
            self.frame.kitPanel.Refresh()

    def OnChooseFile(self, event):
        wildcard = "PNG images (*.png)|*.png|" \
                   "BMP images (*.bmp)|*.bmp"

        kit = self.getKit()
        currdir = kit.foldername

        dlg = wx.FileDialog(
            self, message="Choose a file", defaultDir=currdir, 
            defaultFile="", wildcard=wildcard, style=wx.OPEN | wx.CHANGE_DIR
            )

        # Show the dialog and retrieve the user response. If it is the OK response, 
        # process the data.
        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            files = dlg.GetPaths()
            if len(files)>0:
                newfile = os.path.normcase(files[0])
                foldername = "%s\\%s" % (os.path.split(kit.foldername)[0], kit.shortsKey)
                # make relative path
                self.SetStringSelection(makeRelativePath(newfile, foldername))

                # add kit to modified set
                self.frame.addKitToModified()

        dlg.Destroy()


"""
A dir choice with label
"""
class MyPartFolder(MyNumbersFile):

    def __init__(self, parent, attribute, labelText, value, rootPath, frame, kit=None):
        wx.Panel.__init__(self, parent, -1)
        self.rootPath = rootPath
        self.frame = frame
        self.kit = kit
        self.att = attribute
        self.label = wx.StaticText(self, -1, labelText, size=(140,-1), style=wx.ALIGN_RIGHT)
        self.label.SetBackgroundColour(wx.Colour(230,230,230))
        font = wx.Font(10, wx.SWISS, wx.NORMAL, wx.NORMAL)
        self.label.SetFont(font)
        self.label.SetSize(self.label.GetBestSize())

        self.text = wx.TextCtrl(self, -1, "", size=(130,-1))
        self.text.SetEditable(False)
        self.button = wx.Button(self, -1, "undef", size=(60,1))
        self.fileButton = wx.Button(self, -1, "...", size=(30,1))

        self.sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.sizer.Add(self.button, 0, wx.RIGHT | wx.EXPAND, border=10)
        self.sizer.Add(self.label, 0, wx.RIGHT | wx.ALIGN_CENTER_VERTICAL, border=10)
        self.sizer.Add(self.text, 0, wx.EXPAND)
        self.sizer.Add(self.fileButton, 0, wx.EXPAND)

        # by default the kit panel is not refreshed on selection change
        self.refreshOnChange = False

        # bind events
        #self.text.Bind(wx.EVT_CHOICE, self.OnSelect)
        self.button.Bind(wx.EVT_BUTTON, self.OnUndef)
        self.fileButton.Bind(wx.EVT_BUTTON, self.OnChooseFile)

        self.SetSizer(self.sizer)
        self.Layout()

    def SetStringSelection(self, str=None):
        kit = self.getKit()
        if str == None:
            try: str = kit.attributes[self.att]
            except KeyError: str = "undefined"
        self.text.SetValue(str)
        kit.attributes[self.att] = str
        if self.refreshOnChange:
            self.frame.kitPanel.Refresh()

    def SetUndef(self):
        kit = self.getKit()
        self.text.SetValue("")
        try:
            del kit.attributes[self.att] 
        except AttributeError:
            pass
        except KeyError:
            pass
        if self.refreshOnChange:
            self.frame.kitPanel.Refresh()

    def OnChooseFile(self, event):
        kit = self.getKit()
        tokens = os.path.split(kit.foldername)
        items = [d for d in os.listdir(tokens[0])
                    if os.path.isdir("%s/%s" % (tokens[0],d)) and d[0] in ['p','g']]
        defaultItem = kit.attributes.get(self.att)
        dlg = wx.SingleChoiceDialog(
                self, 'Select the kit part folder', 'Folder selector', items,
                wx.CHOICEDLG_STYLE
                )
        if defaultItem in items: dlg.SetSelection(items.index(defaultItem))
        if dlg.ShowModal() == wx.ID_OK:
            path = dlg.GetStringSelection()
            print "You selected: %s" % path
            if path:
                newpath = os.path.normcase(path)
                self.SetStringSelection(newpath)

                # add kit to modified set
                self.frame.addKitToModified()

        dlg.Destroy()

"""
A file choice with label
"""
class MyMaskFile(MyPartFolder):

    def SetStringSelection(self, str=None):
        kit = self.getKit()
        if not str: str = kit.attributes.get(self.att,"")
        self.text.SetValue(str)
        kit.attributes[self.att] = str
        if self.refreshOnChange:
            self.frame.kitPanel.Refresh()

    def SetUndef(self):
        kit = self.getKit()
        self.text.SetValue("")
        try:
            del kit.attributes[self.att]
        except AttributeError:
            pass
        except KeyError:
            pass
        if self.refreshOnChange:
            self.frame.kitPanel.Refresh()

    def OnChooseFile(self, event):
        wildcard = "PNG images (*.png)|*.png|" \
                   "BMP images (*.bmp)|*.bmp"

        kit = self.getKit()
        curvalue = kit.attributes.get(self.att)
        if not curvalue:
            defaultDir = self.frame.gdbPath + "/uni/masks"
        else:
            # check to see if the file exists relative to kit dir
            fullpath = "%s/%s" % (kit.foldername, curvalue)
            if os.path.exists(fullpath):
                defaultDir = os.path.split(fullpath)[0]
            else:
                fullpath = "%s/uni/masks/%s" % (self.frame.gdbPath, curvalue)
                if os.path.exists(fullpath):
                    defaultDir = os.path.split(fullpath)[0]
                else:
                    defaultDir = self.frame.gdbPath + "/uni/masks"

        defaultFile = kit.attributes.get(self.att,DEFAULT_MASK)

        dlg = wx.FileDialog(
            self, message="Choose a file", defaultDir=defaultDir, 
            defaultFile="", wildcard=wildcard, style=wx.OPEN | wx.CHANGE_DIR
            )

        # Show the dialog and retrieve the user response. If it is the OK response, 
        # process the data.
        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            files = dlg.GetPaths()
            if len(files)>0:
                newfile = os.path.normcase(files[0])
                kitBasedir = os.path.normcase(kit.foldername)
                standardBasedir = os.path.normcase(self.frame.gdbPath + "/uni/masks")
                # make relative path
                relpath1 = makeRelativePath(newfile,kitBasedir)
                relpath2 = makeRelativePath(newfile,standardBasedir)
                if len(relpath1)<=len(relpath2):
                    self.SetStringSelection(relpath1)
                else:
                    self.SetStringSelection(relpath2)

                # add kit to modified set
                self.frame.addKitToModified(kit)

        dlg.Destroy()


"""
A file choice with label
"""
class MyOverlayFile(MyPartFolder):

    def SetStringSelection(self, str=None):
        kit = self.getKit()
        if not str: str = kit.attributes.get(self.att,"")
        self.text.SetValue(str)
        kit.attributes[self.att] = str
        if self.refreshOnChange:
            self.frame.kitPanel.Refresh()

    def SetUndef(self):
        kit = self.getKit()
        self.text.SetValue("")
        try:
            del kit.attributes[self.att]
        except AttributeError:
            pass
        except KeyError:
            pass
        if self.refreshOnChange:
            self.frame.kitPanel.Refresh()

    def OnChooseFile(self, event):
        wildcard = "PNG images (*.png)|*.png|" \
                   "BMP images (*.bmp)|*.bmp"

        kit = self.getKit()
        curvalue = kit.attributes.get(self.att)
        if not curvalue:
            defaultDir = self.frame.gdbPath + "/uni/overlay"
        else:
            # check to see if the file exists relative to kit dir
            fullpath = "%s/%s" % (kit.foldername, curvalue)
            if os.path.exists(fullpath):
                defaultDir = os.path.split(fullpath)[0]
            else:
                fullpath = "%s/uni/overlay/%s" % (self.frame.gdbPath, curvalue)
                if os.path.exists(fullpath):
                    defaultDir = os.path.split(fullpath)[0]
                else:
                    defaultDir = self.frame.gdbPath + "/uni/overlay"

        dlg = wx.FileDialog(
            self, message="Choose a file", defaultDir=defaultDir, 
            defaultFile="", wildcard=wildcard, style=wx.OPEN | wx.CHANGE_DIR
            )

        # Show the dialog and retrieve the user response. If it is the OK response, 
        # process the data.
        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            files = dlg.GetPaths()
            if len(files)>0:
                newfile = os.path.normcase(files[0])
                kitBasedir = os.path.normcase(kit.foldername)
                standardBasedir = os.path.normcase(self.frame.gdbPath + "/uni/overlay")
                # make relative path
                relpath1 = makeRelativePath(newfile,kitBasedir)
                relpath2 = makeRelativePath(newfile,standardBasedir)
                if len(relpath1)<=len(relpath2):
                    self.SetStringSelection(relpath1)
                else:
                    self.SetStringSelection(relpath2)

                # add kit to modified set
                self.frame.addKitToModified(kit)

        dlg.Destroy()


"""
A panel with kit texture
"""
class KitPanel(wx.Panel):
    def __init__(self, parent, frame=None):
        wx.Panel.__init__(self, parent, -1, size=(512, 256))
        self.SetBackgroundColour(wx.Color(180,180,200))
        self.frame = frame
        self.kit = None
        self.kitImg = None

        # bind events
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_RIGHT_UP, self.OnPick)
        self.Bind(wx.EVT_KEY_DOWN, self.frame.OnKeyDown)
        self.Bind(wx.EVT_KEY_UP, self.frame.OnKeyUp)
        #self.Bind(wx.EVT_MOUSE_EVENTS, self.OnMouse)

    def OnMouse(self, event):
        if event.Entering:
            self.SetFocus()
        event.Skip()

    def OnPick(self, evt):
        if self.kit is None or self.kit.teamId == -1:
            return
        if self.frame.shiftDown:
            #print 'ShortsColorPick: %d,%d' % (evt.X, evt.Y)
            x, y = evt.X, evt.Y
            r = self.kitImg.GetRed(x,y)
            g = self.kitImg.GetGreen(x,y)
            b = self.kitImg.GetBlue(x,y)
            #print 'color: %02X%02X%02X' % (r,g,b)
            self.frame.shortsCS.ModifyColour(wx.Color(r,g,b))
        else:
            #print 'MainColorPick: %d,%d' % (evt.X, evt.Y)
            x, y = evt.X, evt.Y
            r = self.kitImg.GetRed(x,y)
            g = self.kitImg.GetGreen(x,y)
            b = self.kitImg.GetBlue(x,y)
            #print 'color: %02X%02X%02X' % (r,g,b)
            self.frame.radarCS.ModifyColour(wx.Color(r,g,b))

    def getPartFile(self, kit, part):
        partFolder = kit.attributes.get("%s.folder" % part)
        if partFolder: foldername = "%s/%s" % (os.path.split(kit.foldername)[0], partFolder)
        else: foldername = kit.foldername
        filename = "%s/%s.png" % (foldername,part)
        if os.path.exists(filename):
            return filename
        filename = "%s/%s.bmp" % (foldername,part)
        if os.path.exists(filename):
            return filename
        filename = "%s/kit.png" % foldername
        if os.path.exists(filename):
            return filename
        filename = "%s/kit.bmp" % foldername
        if os.path.exists(filename):
            return filename
        return None

    def getOverlayFile(self,kit,overlay):
        # check the team folder first
        filename = os.path.normcase("%s/%s" % (kit.foldername,overlay))
        if os.path.exists(filename): return filename
        # check the standard folder
        filename = os.path.normcase("%s/uni/overlay/%s" % (self.frame.gdbPath,overlay))
        if os.path.exists(filename): return filename
        # overlay file not found
        print >>sys.stderr,("overlay '%s' NOT found for kit: %s" % (overlay,kit.foldername)).encode('utf-8')
        return None

    def getMaskFile(self,kit,mask):
        # check the team folder first
        filename = os.path.normcase("%s/%s" % (kit.foldername,mask))
        if os.path.exists(filename): return filename
        # check the standard folder
        filename = os.path.normcase("%s/uni/masks/%s" % (self.frame.gdbPath,mask))
        if os.path.exists(filename): return filename
        # mask file not found
        print >>sys.stderr,("mask file '%s' NOT found for kit: %s" % (mask,kit.foldername)).encode('utf-8')
        return None


    def drawFiles(self, dc, kit, files):
        drawnSome = False
        for part,file in files:
            if file:
                bmp = wx.Bitmap(file)
                width,height = bmp.GetSize()
                if part=="shirt":
                    self.frame.SetTitle("%s: (%dx%d) kit" % (WINDOW_TITLE,width,height))
                if width != 512 or height != 256:
                    bmp = bmp.ConvertToImage().Scale(
                            512,256).ConvertToBitmap()

                img = bmp.ConvertToImage()
                self.kitImg = img
                #maskfile = self.getMaskFile(kit, self.kit.attributes.get("mask",DEFAULT_MASK))
                #if maskfile and len(files)>1:
                #    maskImg = wx.Bitmap(maskfile).ConvertToImage()
                #    applyColourMask(img, maskImg, MASK_COLORS[part])
                dc.DrawBitmap(img.ConvertToBitmap(),0,0,True)
                drawnSome = True
        #overlay = kit.attributes.get("overlay")
        #if overlay:
        #    overlayfile = self.getOverlayFile(kit,overlay)
        #    if overlayfile:
        #        bmp = wx.Bitmap(overlayfile)
        #        img = bmp.ConvertToImage()
        #        width,height = bmp.GetSize()
        #        if width != 512 or height != 512:
        #            img = img.Scale(512,512)
        #        if not img.HasAlpha() and not img.HasMask():
        #            img.SetMaskColour(0xff,0,0xff) # pink overlay mask color
        #        dc.DrawBitmap(img.ConvertToBitmap(),0,0,True)
        return drawnSome

    def drawKit(self, dc, kit):
        files = [(part,self.getPartFile(kit,part)) 
                for part in ["shirt","shorts","socks"]]
        if files[0][1]==files[1][1] and files[1][1]==files[2][1]: 
            del files[1:]
        hasSomething = self.drawFiles(dc, kit, files)
        if not hasSomething:
            bmp = wx.Bitmap(DEFAULT_PNG)
            dc.DrawBitmap(bmp, 0, 0, True)

    def OnPaint(self, event):
        #try: self.repaint(event)
        self.repaint(event)
        #except Exception,info:
        #    print >>sys.stderr,"Exception in KitPanel.OnPaint: %s" % info

    def repaint(self, event):
        dc = wx.PaintDC(self)

        # disable warning pop-ups
        wx.Log.EnableLogging(False)

        # draw kit
        if self.kit == None or self.kit.teamId == -1:
            self.frame.SetTitle(WINDOW_TITLE)
            bmp = wx.Bitmap(DEFAULT_PNG)
            dc.DrawBitmap(bmp, 0, 0, True)
            return event.Skip()
        else:
            self.drawKit(dc, self.kit)

        # draw some overlay items
        kit = self.kit
        p = overlayPositions

        plRect = wx.Rect(64*4,128,64,128)  # number
        gkRect = wx.Rect(64*1,0,64,128)
        plFrontRect = wx.Rect(320+32*4,64,32,64)  # front number
        gkFrontRect = wx.Rect(320+32*1,0,32,64)
        plShortsRect = wx.Rect(320+32*4,128+64,32,64) # shorts number
        gkShortsRect = wx.Rect(320+32*1,128+0,32,64)
        if kit.isKeeper:
            rect = gkRect
            frontRect = gkFrontRect
            shortsRect = gkShortsRect
        else:
            rect = plRect
            frontRect = plFrontRect
            shortsRect = plShortsRect

        # determine shirt type
        try: 
            stp = getKitTemplateType(kit.attributes["model"])
        except KeyError:
            stp = "pes2012"

        kitKey = os.path.split(kit.foldername)[1]
        if os.path.exists(kit.foldername + "/config.txt"):
            readAttributes(kit)

        # render numbers
        numbers = "%s/%s" % (kit.foldername, "numbers.png")
        if os.path.exists(numbers):
            x,y = p[stp]["numbers"]
            try: numY = int(kit.attributes.get("number.y",12))
            except ValueError: numY = 12
            y -= int((numY-12)*2.5)
            try: numSize = int(kit.attributes.get("number.size",12))
            except ValueError: numSize = 12
            osx, osy = p[stp]["num-width"], p[stp]["num-height"]
            sx = int(osx * (0.5+(4+numSize)/40.0))
            sy = int(osy * (0.5+(4+numSize)/40.0))
            x += 0.5*(osx-sx)
            y += 0.5*(osy-sy)

            bmp = wx.Bitmap(numbers).GetSubBitmap(rect)
            scaledbmp = wx.BitmapFromImage(
                    bmp.ConvertToImage().ResampleBicubic(sx,sy))
            dc.DrawBitmap(scaledbmp, x, y, True)

        # render number on the chest
        numbers = "%s/%s" % (kit.foldername, "numbers.png")
        if os.path.exists(numbers):
            snl = kit.attributes.get("front.number.show",None)
            if snl=='1' or isNationalKit(self.kit.teamId):
                if not snl: snl = "1"
                try: coords = p[stp]['chest-%s' % snl]
                except KeyError: coords = None
                if coords:
                    x, y = coords
                    try: numY = int(kit.attributes.get("front.number.y",14))
                    except ValueError: numY = 14
                    y -= int((numY-14)*p[stp]['front-number-y-mult'])
                    try: numX = int(kit.attributes.get("front.number.x",14))
                    except ValueError: numX = 14
                    x -= int((numX-14)*p[stp]['front-number-x-mult'])
                    try: numSize = int(kit.attributes.get("front.number.size",12))
                    except ValueError: numSize = 12
                    osx, osy = p[stp]["chest-num-width"], p[stp]["chest-num-height"]
                    smult = p[stp]["front-number-size-mult"]
                    sx = int(osx * (0.5+(2+numSize*smult)/28.0))
                    sy = int(osy * (0.5+(2+numSize*smult)/28.0))
                    x += 0.5*(osx-sx)
                    y += 0.5*(osy-sy)

                    bmp = wx.Bitmap(numbers).GetSubBitmap(frontRect)
                    scaledbmp = wx.BitmapFromImage(
                            bmp.ConvertToImage().ResampleBicubic(sx,sy))
                    dc.DrawBitmap(scaledbmp, x, y, True)

        # render number on the shorts
        numbers = "%s/%s" % (kit.foldername, "numbers.png")
        if os.path.exists(numbers):
            shortsLoc = kit.attributes.get("shorts.number.location",None)
            if not shortsLoc or shortsLoc == "left":
                x, y = p[stp]['shorts-left']
                try: numY = int(kit.attributes.get("shorts.number.y",14))
                except ValueError: numY = 14
                y -= int((numY-14)*p[stp]['shorts-number-y-mult'])
                try: numX = int(kit.attributes.get("shorts.number.x",14))
                except ValueError: numX = 14
                x -= int((numX-14)*p[stp]['shorts-number-x-mult'])
                try: numSize = int(kit.attributes.get("shorts.number.size",12))
                except ValueError: numSize = 12
                osx, osy = p[stp]["shorts-num-width"], p[stp]["shorts-num-height"]
                smult = p[stp]["shorts-number-size-mult"]
                sx = int(osx * (0.5+(2+numSize*smult)/28.0))
                sy = int(osy * (0.5+(2+numSize*smult)/28.0))
                x += 0.5*(osx-sx)
                y += 0.5*(osy-sy)
                bmp = wx.Bitmap(numbers).GetSubBitmap(shortsRect)
                scaledbmp = wx.BitmapFromImage(
                        bmp.ConvertToImage().ResampleBicubic(sx,sy))
                dc.DrawBitmap(scaledbmp, x, y, True)

            if shortsLoc == "right":
                x, y = p[stp]['shorts-right']
                try: numY = int(kit.attributes.get("shorts.number.y",14))
                except ValueError: numY = 14
                y -= int((numY-14)*p[stp]['shorts-number-y-mult'])
                try: numX = int(kit.attributes.get("shorts.number.x",14))
                except ValueError: numX = 14
                x += int((numX-14)*p[stp]['shorts-number-x-mult'])
                try: numSize = int(kit.attributes.get("shorts.number.size",12))
                except ValueError: numSize = 12
                osx, osy = p[stp]["shorts-num-width"], p[stp]["shorts-num-height"]
                smult = p[stp]["shorts-number-size-mult"]
                sx = int(osx * (0.5+(2+numSize*smult)/28.0))
                sy = int(osy * (0.5+(2+numSize*smult)/28.0))
                x += 0.5*(osx-sx)
                y += 0.5*(osy-sy)
                bmp = wx.Bitmap(numbers).GetSubBitmap(shortsRect)
                scaledbmp = wx.BitmapFromImage(
                        bmp.ConvertToImage().ResampleBicubic(sx,sy))
                dc.DrawBitmap(scaledbmp, x, y, True)

        # render name
        font = "%s/%s" % (kit.foldername, "font.png")
        if not os.path.exists(font):
            font = "%s/%s" % (kit.foldername, "font.bmp")
        if os.path.exists(font):
            try: nameType = kit.attributes["name.shape"]
            except KeyError: nameType = "type1"
            try: nameShow = kit.attributes["name.show"]
            except KeyError: nameShow = "1"

            if nameType not in ["type1","type2","type3","type4"]:
                nameType = "type1"

            if nameShow == "1":
                rect1 = wx.Rect(0,0,20,32)
                rect2 = wx.Rect(20,0,20,32)
                rect3 = wx.Rect(40,0,20,32)
                bmp = wx.Bitmap(font).GetSubBitmap(rect1)
                img = bmp.ConvertToImage().ResampleBicubic(p[stp]["font-width"],p[stp]["font-height"])
                if nameType == "type2": img = img.Rotate(math.pi/18,wx.Point(27,p[stp]["font-height"]-1))
                elif nameType == "type3": img = img.Rotate(math.pi/12,wx.Point(27,p[stp]["font-height"]-1))
                elif nameType == "type4": img = img.Rotate(math.pi/6,wx.Point(27,p[stp]["font-height"]-1))
                bmp = img.ConvertToBitmap()
                x, y = p[stp]["nameA-%s" % nameType]
                try: nameY = int(kit.attributes.get("name.y",24))
                except ValueError: nameY = 24
                y -= int((nameY-24)*p[stp]['name-y-mult'])
                dc.DrawBitmap(bmp, x, y, True)
                bmp = wx.Bitmap(font).GetSubBitmap(rect2)
                bmp = bmp.ConvertToImage().ResampleBicubic(p[stp]["font-width"],p[stp]["font-height"]).ConvertToBitmap()
                x, y = p[stp]["nameB-%s" % nameType]
                y -= int((nameY-24)*p[stp]['name-y-mult'])
                dc.DrawBitmap(bmp, x, y, True)
                bmp = wx.Bitmap(font).GetSubBitmap(rect3)
                img = bmp.ConvertToImage().ResampleBicubic(p[stp]["font-width"],p[stp]["font-height"])
                if nameType == "type2": img = img.Rotate(-math.pi/18,wx.Point(0,p[stp]["font-height"]-1))
                elif nameType == "type3": img = img.Rotate(-math.pi/12,wx.Point(0,p[stp]["font-height"]-1))
                elif nameType == "type4": img = img.Rotate(-math.pi/6,wx.Point(0,p[stp]["font-height"]-1))
                bmp = img.ConvertToBitmap()
                x, y = p[stp]["nameC-%s" % nameType]
                y -= int((nameY-24)*p[stp]['name-y-mult'])
                dc.DrawBitmap(bmp, x, y, True)

        # re-enable warning pop-ups
        wx.Log.EnableLogging(True)

        nameText, numberText = "ABC", "9"
        if self.kit.isKeeper:
            numberText = "1"

        # shirt name
        try:
            colorName = MakeRGBAColor(self.kit.attributes["shirt.name"])
            if colorName.alpha != 0:
                dc.SetFont(wx.Font(14, wx.SWISS, wx.NORMAL, wx.BOLD, False))
                dc.SetTextForeground(colorName.color)
                try:
                    if self.kit.attributes["name.shape"] == "curved":
                        dc.DrawRotatedText(nameText[0], 238, 15, 7)
                        dc.DrawText(nameText[1], 253, 15)
                        dc.DrawRotatedText(nameText[2], 268, 15, -7)
                    else:
                        dc.DrawText(nameText, 238, 15)
                except KeyError:
                    dc.DrawText(nameText, 238, 15)
        except KeyError:
            pass

        # shirt number
        try:
            colorNumber = MakeRGBAColor(self.kit.attributes["shirt.number"])
            if colorNumber.alpha != 0:
                dc.SetFont(wx.Font(42, wx.SWISS, wx.NORMAL, wx.BOLD, False))
                dc.SetTextForeground(colorNumber.color)
                dc.DrawText(numberText, 244, 46)
        except KeyError:
            pass

        # shirt front number (for national teams)
        if self.kit.teamId < 64:
            try:
                colorNumber = MakeRGBAColor(self.kit.attributes["shirt.number"])
                if colorNumber.alpha != 0:
                    dc.SetFont(wx.Font(20, wx.SWISS, wx.NORMAL, wx.BOLD, False))
                    dc.SetTextForeground(colorNumber.color)
                    dc.DrawText(numberText, 109, 66)
            except KeyError:
                pass

        # shorts number(s)
        try:
            colorShorts = MakeRGBAColor(self.kit.attributes["shorts.number"])
            if colorShorts.alpha != 0:
                dc.SetFont(wx.Font(20, wx.SWISS, wx.NORMAL, wx.BOLD, False))
                dc.SetTextForeground(colorShorts.color)
                try:
                    shortsNumPos = self.kit.attributes["shorts.number.location"];
                    if shortsNumPos == "off":
                        positions = []
                    elif shortsNumPos == "right":
                        positions = [(40,205)]
                    else:
                        positions = [(120,205)]
                except KeyError:
                    positions = [(120,205)]
                for pos in positions:
                    dc.DrawText(numberText, pos[0], pos[1])
        except KeyError:
            pass


class Kit:
    def __init__(self, foldername):
        # create a kit with undefined attributes 
        self.foldername = os.path.normcase(foldername)
        self.attributes = dict()
        self.isKeeper = False # flag to indicate a GK kit
        self.attribRead = False # flag to indicate that attributes were already read
        self.teamId = -1

        # set goalkeeper flag
        path, kitKey = os.path.split(foldername)
        if kitKey[0] == 'g' or kitKey[:6] == 'euro-g':
            self.isKeeper = True


class GDBTree(wx.TreeCtrl):
    def __init__(self, parent, style, frame, gdbPath=""):
        wx.TreeCtrl.__init__(self, parent, -1, style=style)
        self.gdbPath = gdbPath
        self.frame = frame

        self.root = self.AddRoot("GDB")
        self.SetPyData(self.root, None)

        self.teamMap = dict()
        self.reverseTeamMap = dict()

        # bind events
        self.Bind(wx.EVT_KEY_DOWN, self.frame.OnKeyDown)
        self.Bind(wx.EVT_KEY_UP, self.frame.OnKeyUp)
        self.Bind(wx.EVT_TREE_KEY_DOWN, self.OnKeyDown)
        self.Bind(wx.EVT_TREE_SEL_CHANGED, self.OnSelChanged)


    """
    Shows a warning window, with a choice of saving changes,
    discarding them, or cancelling the operation.
    """
    def cancelledOnSaveChanges(self):
        if len(self.frame.modified) > 0:
            # figure out what to do with changes: Save or Discard
            dlg = wx.MessageDialog(self, """You haven't saved your changes.
Do you want to save them?""",
                    "Save or Discard changes",
                    wx.YES_NO | wx.CANCEL | wx.ICON_INFORMATION)
            retValue = dlg.ShowModal()
            dlg.Destroy()

            if retValue == wx.ID_YES:
                # save the changes first
                self.frame.saveChanges(False)
                pass
            elif retValue == wx.ID_CANCEL:
                # cancel the operation
                return True

        self.frame.modified.clear()
        self.frame.SetStatusText("Modified kits: 0")
        return False

    
    def OnRefresh(self, event):
        if self.cancelledOnSaveChanges():
            return

        self.updateTree()
        self.frame.modified.clear()
        self.frame.SetStatusText("Modified kits: 0");
        self.frame.selectKit(None)
        print "GDB-tree updated."


    def OnKeyDown(self, event):
        key = event.GetKeyCode()
        item = self.GetSelection()
        if key == wx.WXK_RETURN:
            if self.IsExpanded(item):
                self.Collapse(item)
            else:
                self.Expand(item)


    def OnSelChanged(self, event):
        try:
            item = event.GetItem()
            #print "OnSelChanged: %s" % self.GetItemText(item)
            kit = self.GetPyData(item)
            self.frame.selectKit(kit)

        except wx._core.PyAssertionError:
            pass

    """
    Creates new Kit object
    """
    def makeKit(self, path):
        kit = Kit(path)
        try: 
            foldername = os.path.normcase(os.path.split(path)[0])
            kit.teamId = self.reverseTeamMap[foldername][0]
            kit.shortsKey = os.path.split(path)[1]
        except KeyError: kit.teamId = -1
        return kit

    """
    Recursivelly adds specified path.
    """
    def addDir(self, node, path, inmap=False, cinmap=False):
        if os.path.isdir(path):
            child = self.AppendItem(node, "%s" % os.path.split(path)[1])
            kit = self.makeKit(path)
            self.SetPyData(child, kit)
            self.SetItemImage(child, self.fldridx, wx.TreeItemIcon_Normal)
            self.SetItemImage(child, self.fldropenidx, wx.TreeItemIcon_Expanded)

            inmap = inmap or self.reverseTeamMap.has_key(os.path.normcase(path))

            teamIds = self.reverseTeamMap.get(os.path.normcase(path))
            if teamIds:
                self.SetItemText(child, 
                    "%s (%s)" % (self.GetItemText(child), 
                        ','.join(str(x) for x in teamIds)))

            dirs = ["%s/%s" % (path,item) for item in os.listdir(path)]
            dirs.sort()
            for dir in dirs:
                cinmap = self.addDir(child,dir,inmap) or cinmap
                #print dir

            if not inmap and not cinmap:
                self.Delete(child)

            return cinmap or inmap


    def updateTree(self):
        self.CollapseAndReset(self.root)

        gdbPath = self.gdbPath
        if gdbPath is None:
            gdbPath = ""

        isz = (16,16)
        il = wx.ImageList(isz[0], isz[1])
        self.fldridx     = il.Add(wx.ArtProvider_GetBitmap(wx.ART_FOLDER,      wx.ART_OTHER, isz))
        self.fldropenidx = il.Add(wx.ArtProvider_GetBitmap(wx.ART_FILE_OPEN,   wx.ART_OTHER, isz))
        self.fileidx     = il.Add(wx.ArtProvider_GetBitmap(wx.ART_NORMAL_FILE, wx.ART_OTHER, isz))

        self.SetImageList(il)
        self.il = il

        self.SetItemImage(self.root, self.fldridx, wx.TreeItemIcon_Normal)
        self.SetItemImage(self.root, self.fldropenidx, wx.TreeItemIcon_Expanded)

        # Populate the tree control with content from GDB.
        # The idea here is to only add those files/folders to the tree, which
        # actually are recognized and processed by Kitserver, and leave everything
        # else out of the tree control. (One exception to this rule is: config.txt
        # file in each team folder. Kitserver does process it, but we are not gonna
        # show that file in the tree.)

        try: 
            # read map.txt
            self.teamMap.clear()
            self.reverseTeamMap.clear()
            try: map = codecs.open(gdbPath + "/uni/map.txt","rt","utf-8")
            except IOError: pass
            else:
                for line in map:
                    # strip off comments
                    comm = line.find("#")
                    if comm > -1:
                        line = line[:comm]

                    # strip off any remaining white space
                    line = line.strip()
                    if len(line)==0: continue

                    # work around BOM-bug
                    if line[0]==u'\ufeff':
                        line = line[1:]

                    tok = line.split(',',1)
                    if len(tok)==2:
                        try:
                            id = int(tok[0])
                        except ValueError:
                            try:
                                id = int(tok[0],16)
                            except ValueError:
                                # skip this line
                                continue
                        val = tok[1].strip()
                        if val[0]=='"' and val[-1]=='"': val = val[1:-1]
                        folder = os.path.normcase(self.gdbPath + "/uni/" + val)
                        self.teamMap[id] = folder
                        self.reverseTeamMap.setdefault(folder,[]).append(id)

                map.close()

            # add all the dirs
            teamDirs = self.addDir(self.root, gdbPath + "/uni")
        except IndexError, ex:
            dlg = wx.MessageDialog(self, """PROBLEM: GDB Manager is unable to read the
contents of your GDB. You selected: 

%s

Perhaps, you accidently selected a wrong folder. 
Please try choosing a different one.""" % self.gdbPath,
                    "GDB Manager ERROR",
                    wx.OK | wx.ICON_INFORMATION)
            dlg.ShowModal()
            dlg.Destroy()

            # trigger folder selection
            self.frame.OnSetFolder(None)

        # show the contents of GDB
        self.Expand(self.root)
        self.SetFocus()
        self.SelectItem(self.root)


class MyFrame(wx.Frame):
    def __init__(self, parent, id, title):
        global maxModel, techfitModels
        wx.Frame.__init__(self, parent, id, title, size=(FRAME_WIDTH, FRAME_HEIGHT))
        self.gdbPath = u"C:\\"
        self.kservCfgPath = ""

        # create a dictionary to keep track of modified kits
        self.modified = {}

        # status bar
        self.CreateStatusBar()
        self.SetStatusText("Modified kits: 0")

        self.shiftDown = False

        # Kit database folder
        try:
            cfg = codecs.open(CONFIG_FILE, "rt","utf-8")
            d = yaml.load(cfg.read())
            self.gdbPath = d.get('gdbPath')
            techfitModels = d.get('techfitModels', techfitModels)
            maxModel = d.get('maxModel', maxModel)
            print ("self.gdbPath = {%s}" % self.gdbPath).encode('utf-8')
            print "techfitModels = %s" % techfitModels
            print "maxModel = %s" % maxModel
            #self.kservCfgPath = cfg.readline().strip()
            #print "self.kservCfgPath = {%s}" % self.kservCfgPath
            cfg.close()
        except IOError:
            pass
        #    self.OnSetFolder(None)
        #if self.gdbPath is None:
        #    self.OnSetFolder(None)

        # Create widgets
        ##################

        splitter = wx.SplitterWindow(self, -1, style=wx.SP_3D)

        p3 = wx.Panel(splitter, -1)
        p3.SetAutoLayout(0);

        # right parent panel
        p2 = scrolled.ScrolledPanel(p3, -1)
        p2.SetAutoLayout(0);
        p2.SetupScrolling(scroll_x=False);

        # current kit panel
        kitPanelHolder = wx.Panel(p3, -1)
        self.kitPanel = KitPanel(kitPanelHolder, self)

        # model choice
        modellist = ["undefined"]
        for id in range(maxModel+1):
            modellist.append(id)
        self.model = MyList(p2, "model", "Kit model:", modellist, self)
        self.model.refreshOnChange = True

        # collar choice
        self.collar = MyList(p2, "collar", "Collar:", ["undefined", "0", "1", "2"], self)

        # name-show choice
        self.nameShow = MyList(p2, "name.show", "Name show:", ["undefined", "0", "1"], self)
        self.nameShow.refreshOnChange = True

        # name-shape choice
        self.nameShape = MyList(p2, "name.shape", "Name shape:", ["undefined", "type1", "type2", "type3", "type4"], self)
        self.nameShape.refreshOnChange = True

        # name-size choice
        values = range(3,33)
        values.reverse()
        self.nameSize = MyList(p2, "name.size", "Name size:", 
                ["undefined"]+values, self)
        self.nameSize.refreshOnChange = True

        # name-y choice
        values = range(0,29)
        values.reverse()
        self.nameY = MyList(p2, "name.y", "Name Y-pos:", 
                ["undefined"]+values, self)
        self.nameY.refreshOnChange = True

        # logo-location choice
        #self.logoLocation = MyList(p2, "logo.location", "Logo location:", ["undefined", "off", "top", "bottom"], self)

        # numbers choice
        #self.numbers = MyNumbersFile(p2, "numbers", "Numbers:", "undefined", self.gdbPath, self)
        #self.numbers.refreshOnChange = True

        # palette choice
        #self.numpal = MyShortsNumPalFile(p2, "shorts.num-pal.%s", "Palette:", "undefined", self.gdbPath, self)
        #self.numpal.refreshOnChange = True

        # number-size choice
        values = range(3,41)
        values.reverse()
        self.numberSize = MyList(p2, "number.size", "Number size:", 
                ['undefined']+values, self)
        self.numberSize.refreshOnChange = True

        # number-y choice
        values = range(0,33)
        values.reverse()
        self.numberY = MyList(p2, "number.y", "Number Y-pos:", 
                ['undefined']+values, self)
        self.numberY.refreshOnChange = True

        # front-number-show choice
        self.frontNumberShow = MyList(p2, "front.number.show", "Front number show:", 
                ["undefined", "0", "1"], self)
        self.frontNumberShow.refreshOnChange = True

        # front-number-size choice
        values = range(3,33)
        values.reverse()
        self.frontNumberSize = MyList(p2, "front.number.size", "Front number size:", 
                ["undefined"]+values, self)
        self.frontNumberSize.refreshOnChange = True

        # front-number-x choice
        values = range(0,29)
        values.reverse()
        self.frontNumberX = MyList(p2, "front.number.x", "Front number X-pos:", 
                ["undefined"]+values, self)
        self.frontNumberX.refreshOnChange = True

        # front-number-y choice
        values = range(0,29)
        values.reverse()
        self.frontNumberY = MyList(p2, "front.number.y", "Front number Y-pos:", 
                ["undefined"]+values, self)
        self.frontNumberY.refreshOnChange = True

        # shorts-num-location choice
        self.shortsNumLocation = MyList(p2, "shorts.number.location", "Shorts number location:", ["undefined", "off", "left", "right"], self)
        self.shortsNumLocation.refreshOnChange = True

        # shorts-number-size choice
        values = range(3,33)
        values.reverse()
        self.shortsNumberSize = MyList(p2, "shorts.number.size", "Shorts number size:", 
                ["undefined"]+values, self)
        self.shortsNumberSize.refreshOnChange = True

        # shorts-number-x choice
        values = range(0,29)
        values.reverse()
        self.shortsNumberX = MyList(p2, "shorts.number.x", "Shorts number X-pos:", 
                ["undefined"]+values, self)
        self.shortsNumberX.refreshOnChange = True

        # shorts-number-y choice
        values = range(0,29)
        values.reverse()
        self.shortsNumberY = MyList(p2, "shorts.number.y", "Shorts number Y-pos:", 
                ["undefined"]+values, self)
        self.shortsNumberY.refreshOnChange = True

        # sleeve-patch choice
        #values = range(0,20)
        #self.sleevePatch = MyList(p2, "sleeve.patch", "Sleeve patch:", 
        #        ["undefined"]+values, self)
        #self.sleevePatch.refreshOnChange = True

        # sleeve-patch-left-pos-long choice
        values = range(0,14)
        self.sleevePatchLeftPosLong = MyList(
            p2, "sleeve.patch.left.pos.long", 
            "Left long sleeves patch position:", 
            ["undefined"]+values, self)
        self.sleevePatchLeftPosLong.refreshOnChange = True

        # sleeve-patch-left-pos-short choice
        values = range(0,14)
        self.sleevePatchLeftPosShort = MyList(
            p2, "sleeve.patch.left.pos.short", 
            "Left short sleeves patch position:", 
            ["undefined"]+values, self)
        self.sleevePatchLeftPosShort.refreshOnChange = True

        # sleeve-patch-right-pos-long choice
        values = range(0,14)
        self.sleevePatchRightPosLong = MyList(
            p2, "sleeve.patch.right.pos.long", 
            "Right long sleeves patch position:", 
            ["undefined"]+values, self)
        self.sleevePatchRightPosLong.refreshOnChange = True

        # sleeve-patch-right-pos-short choice
        values = range(0,14)
        self.sleevePatchRightPosShort = MyList(
            p2, "sleeve.patch.right.pos.short", 
            "Right short sleeves patch position:", 
            ["undefined"]+values, self)
        self.sleevePatchRightPosShort.refreshOnChange = True

        #self.checkShortsCombos = wx.Button(p2, -1, "Check other shorts combinations");
        #self.shortsKeys = MyKeyList(p2, "With shorts from", [], self);

        # main (radar) color
        self.radarCS = KitColourSelect(
            p2, "main.color", "Main (radar) color:", self)

        # shorts color
        self.shortsCS = KitColourSelect(
            p2, "shorts.color", "Shorts color:", self)

        # second color
        self.secondCS = KitColourSelect(
            p2, "second.color", "Shirt second color:", self)

        # socks color
        self.socksCS = KitColourSelect(
            p2, "socks.color", "Socks color:", self)

        # icon type
        values = range(0,17)
        self.iconType = MyList(
            p2, "icon.type", "Formation icon type:", 
            ["undefined"]+values, self)
        self.iconType.refreshOnChange = True

        # kit decription
        self.desc = MyTextField(p2, "description", "Description:", "undefined", self.gdbPath, self)
        self.desc.refreshOnChange = True

        # shirt.folder choice
        #self.shirtFolder = MyPartFolder(p2, "shirt.folder", "Shirt Folder:", "undefined", self.gdbPath, self)
        #self.shirtFolder.refreshOnChange = True

        # shorts.folder choice
        #self.shortsFolder = MyPartFolder(p2, "shorts.folder", "Shorts Folder:", "undefined", self.gdbPath, self)
        #self.shortsFolder.refreshOnChange = True

        # shorts.folder choice
        #self.socksFolder = MyPartFolder(p2, "socks.folder", "Socks Folder:", "undefined", self.gdbPath, self)
        #self.socksFolder.refreshOnChange = True

        # mask file choice
        #self.maskFile = MyMaskFile(p2, "mask", "Mask:", "undefined", self.gdbPath, self)
        #self.maskFile.refreshOnChange = True

        # overlay file choice
        #self.overlayFile = MyOverlayFile(p2, "overlay", "Overlay:", "undefined", self.gdbPath, self)
        #self.overlayFile.refreshOnChange = True

        # Let user a chance to select folder
        if self.gdbPath is None:
            self.OnSetFolder(None)

        # tree control
        self.tree = GDBTree(splitter, wx.TR_HAS_BUTTONS, self, self.gdbPath)
        self.tree.updateTree()

        # menu
        menubar = wx.MenuBar()

        menu1 = wx.Menu()
        menu1.Append(101, "&GDB folder", 
            "Set/change the location of GDB folder. Your current is: %s" % self.gdbPath)
        menu1.Append(102, "&Save changes", "Save the changes to config.txt files")
        menu1.Append(104, "Show &modified kits", "Show which kits you have modified")
        menu1.AppendSeparator()
        menu1.Append(103, "E&xit", "Exit the program")
        menubar.Append(menu1, "&File")

        menu2 = wx.Menu()
        menu2.Append(202, "&Restore this kit", "Undo changes for this kit")
        menu2.Append(201, "Re&load GDB", "Reload the GDB from disk")
        menubar.Append(menu2, "&Edit")

        #menu4 = wx.Menu()
        #menu4.Append(401, "&Link to kserv.cfg", "Link to mini-kits list defined in kserv.cfg")
        #menubar.Append(menu4, "&Tools")

        menu3 = wx.Menu()
        menu3.Append(301, "&About", "Author and version information")
        menubar.Append(menu3, "&Help")

        self.SetMenuBar(menubar)

        # Create sizers
        #################

        self.rightSizer = wx.BoxSizer(wx.VERTICAL)
        p2.SetSizer(self.rightSizer)

        # Build interface by adding widgets to sizers
        ################################################

        self.rightSizer.Add(self.radarCS, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.shortsCS, 0, wx.LEFT | wx.BOTTOM | wx.ALIGN_CENTER | wx.EXPAND, border=10)

        self.rightSizer.Add(self.secondCS, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.socksCS, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.iconType, 0, wx.LEFT | wx.BOTTOM | wx.ALIGN_CENTER | wx.EXPAND, border=10)

        self.rightSizer.Add(self.model, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.collar, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.nameShow, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.nameShape, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.nameSize, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.nameY, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.numberSize, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.numberY, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.frontNumberShow, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.frontNumberSize, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.frontNumberX, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.frontNumberY, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.shortsNumLocation, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.shortsNumberSize, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.shortsNumberX, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.shortsNumberY, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        #self.rightSizer.Add(self.sleevePatch, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.sleevePatchLeftPosLong, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.sleevePatchLeftPosShort, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.sleevePatchRightPosLong, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)
        self.rightSizer.Add(self.sleevePatchRightPosShort, 0, wx.LEFT | wx.ALIGN_CENTER | wx.EXPAND, border=10)

        self.rightSizer.Add(self.desc, 0, wx.LEFT | wx.TOP | wx.BOTTOM | wx.ALIGN_CENTER | wx.EXPAND, border=10)

        kitPanelSizer = wx.BoxSizer(wx.HORIZONTAL)
        kitPanelHolder.SetSizer(kitPanelSizer)
        kitPanelSizer.Add(self.kitPanel, 0, wx.ALIGN_CENTER | wx.EXPAND, border=0)

        self.mainRightSizer = wx.BoxSizer(wx.VERTICAL)
        p3.SetSizer(self.mainRightSizer)
        self.mainRightSizer.Add(kitPanelHolder, 0, wx.BOTTOM | wx.ALIGN_CENTER, border=10)
        self.mainRightSizer.Add(p2, 1, wx.BOTTOM | wx.ALIGN_CENTER | wx.EXPAND, border=0)

        splitter.SetMinimumPaneSize(80)
        #splitter.SplitVertically(self.tree, p2, -520)
        splitter.SplitVertically(self.tree, p3, -520)

        #self.Layout()

        # Bind events
        self.Bind(wx.EVT_CLOSE, self.OnExit)
        #self.Bind(wx.EVT_SIZE, self.OnResize)

        self.Bind(wx.EVT_MENU, self.OnSetFolder, id=101)
        self.Bind(wx.EVT_MENU, self.OnMenuSave, id=102)
        self.Bind(wx.EVT_MENU, self.OnShowModified, id=104)
        self.Bind(wx.EVT_MENU, self.OnExit, id=103)
        self.Bind(wx.EVT_MENU, self.OnRestore, id=202)
        self.Bind(wx.EVT_MENU, self.tree.OnRefresh, id=201)
        self.Bind(wx.EVT_MENU, self.OnSetKservCfg, id=401)
        self.Bind(wx.EVT_MENU, self.OnAbout, id=301)

        #self.checkShortsCombos.Bind(wx.EVT_BUTTON, self.OnCheckShortsCombos)

    def bindKeyEvents(self, controls):
        for ctrl in controls:
            ctrl.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
            ctrl.Bind(wx.EVT_KEY_UP, self.OnKeyUp)

    def OnKeyDown(self, event):
        key = event.GetKeyCode()
        if key == wx.WXK_SHIFT and not self.shiftDown:
            self.shiftDown = True
            #print 'SHIFT down'
        event.Skip()

    def OnKeyUp(self, event):
        key = event.GetKeyCode()
        if key == wx.WXK_SHIFT:
            self.shiftDown = False
            #print 'SHIFT up'
        event.Skip()


    """
    Shows dialog window to select the GDB folder.
    """
    def OnSetFolder(self, event):
        try:
            if self.tree.cancelledOnSaveChanges():
                return
        except AttributeError:
            # no tree yet. ignore then
            pass

        print "Set/change GDB folder location"
        dlg = wx.DirDialog(self, """Select your GDB folder:
(Folder named "GDB", which is typically located
inside your kitserver folder)""",
                style=wx.DD_DEFAULT_STYLE)

        if dlg.ShowModal() == wx.ID_OK:
            self.gdbPath = dlg.GetPath()
            print "You selected %s" % self.gdbPath

            # clear out kit panel, and disable controls
            self.enableControls(None)
            self.kitPanel.kit = None
            self.kitPanel.Refresh()

            # try to update the tree
            try:
                self.tree.gdbPath = self.gdbPath
                self.tree.updateTree()
            except AttributeError:
                # looks like we don't have a tree yet. 
                # so just rememeber this value for now.
                pass

            # save the value in configuration file
            #print "Saving configuration into gdbm.yaml..."
            print "Saving configuration into gdbm.yaml..."
            try:
                cfg = codecs.open(CONFIG_FILE, "wt", "utf-8")
                d = { 'gdbPath':self.gdbPath,
                      'techfitModels':techfitModels,
                      'maxModel':maxModel }
                print d
                cfg.write(yaml.dump(d))
                cfg.close()
            except IOError:
                # unable to save configuration file
                print "Unable to save configuration file"

        else:
            print "Selection cancelled."

        # destroy the dialog after we're done
        dlg.Destroy()


    def selectKit(self, kit):
        self.enableControls(kit)

        if kit != None:
            readAttributes(kit)
        else:
            self.SetTitle(WINDOW_TITLE)

        # assign this kit to kitPanel
        self.kitPanel.kit = kit
        self.kitPanel.Refresh()

        # update collar
        try:
            self.collar.SetStringSelection(kit.attributes["collar"])
        except:
            self.collar.SetUndef()

        # update model
        try:
            self.model.SetStringSelection(kit.attributes["model"])
        except:
            self.model.SetUndef()

        # update nameShape
        try:
            self.nameShape.SetStringSelection(kit.attributes["name.shape"])
        except:
            self.nameShape.SetUndef()

        # update nameShow
        try:
            self.nameShow.SetStringSelection(kit.attributes["name.show"])
        except:
            self.nameShow.SetUndef()

        # update nameSize
        try:
            self.nameSize.SetStringSelection(kit.attributes["name.size"])
        except:
            self.nameSize.SetUndef()

        # update nameY
        try:
            self.nameY.SetStringSelection(kit.attributes["name.y"])
        except:
            self.nameY.SetUndef()
        # update logoLocation
        #try:
        #    self.logoLocation.SetStringSelection(kit.attributes["logo.location"])
        #except:
        #    self.logoLocation.SetUndef()

        # update numberSize
        try:
            self.numberSize.SetStringSelection(kit.attributes["number.size"])
        except:
            self.numberSize.SetUndef()

        # update numberY
        try:
            self.numberY.SetStringSelection(kit.attributes["number.y"])
        except:
            self.numberY.SetUndef()

        # update frontNumberShow
        try:
            self.frontNumberShow.kit = kit
            self.frontNumberShow.SetStringSelection(kit.attributes["front.number.show"])
        except:
            self.frontNumberShow.SetUndef()
            
        # update frontNumberSize
        try:
            self.frontNumberSize.kit = kit
            self.frontNumberSize.SetStringSelection(kit.attributes["front.number.size"])
        except:
            self.frontNumberSize.SetUndef()

        # update frontNumberX
        try:
            self.frontNumberX.kit = kit
            self.frontNumberX.SetStringSelection(kit.attributes["front.number.x"])
        except:
            self.frontNumberX.SetUndef()

        # update frontNumberY
        try:
            self.frontNumberY.kit = kit
            self.frontNumberY.SetStringSelection(kit.attributes["front.number.y"])
        except:
            self.frontNumberY.SetUndef()

        # update shortsNumLocation
        #try:
        #    self.shortsNumLocation.SetStringSelection(kit.attributes["shorts.number.location"])
        #except:
        #    self.shortsNumLocation.SetUndef()
        try:
            self.shortsNumLocation.kit = kit
            self.shortsNumLocation.SetStringSelection(kit.attributes["shorts.number.location"])
        except:
            self.shortsNumLocation.SetUndef()

        # update shortsNumberSize
        try:
            self.shortsNumberSize.kit = kit
            self.shortsNumberSize.SetStringSelection(kit.attributes["shorts.number.size"])
        except:
            self.shortsNumberSize.SetUndef()

        # update shortsNumberX
        try:
            self.shortsNumberX.kit = kit
            self.shortsNumberX.SetStringSelection(kit.attributes["shorts.number.x"])
        except:
            self.shortsNumberX.SetUndef()

        # update shortsNumberY
        try:
            self.shortsNumberY.kit = kit
            self.shortsNumberY.SetStringSelection(kit.attributes["shorts.number.y"])
        except:
            self.shortsNumberY.SetUndef()

        # update sleevePatch
        #try:
        #    self.sleevePatch.kit = kit
        #    self.sleevePatch.SetStringSelection(kit.attributes["sleeve.patch"])
        #except:
        #    self.sleevePatch.SetUndef()

        # update sleevePatchLeftPosLong
        try:
            self.sleevePatchLeftPosLong.kit = kit
            self.sleevePatchLeftPosLong.SetStringSelection(
                kit.attributes["sleeve.patch.left.pos.long"])
        except:
            self.sleevePatchLeftPosLong.SetUndef()

        # update sleevePatchLeftPosShort
        try:
            self.sleevePatchLeftPosShort.kit = kit
            self.sleevePatchLeftPosShort.SetStringSelection(
                kit.attributes["sleeve.patch.left.pos.short"])
        except:
            self.sleevePatchLeftPosShort.SetUndef()

        # update sleevePatchRightPosLong
        try:
            self.sleevePatchRightPosLong.kit = kit
            self.sleevePatchRightPosLong.SetStringSelection(
                kit.attributes["sleeve.patch.right.pos.long"])
        except:
            self.sleevePatchRightPosLong.SetUndef()

        # update sleevePatchRightPosShort
        try:
            self.sleevePatchRightPosShort.kit = kit
            self.sleevePatchRightPosShort.SetStringSelection(
                kit.attributes["sleeve.patch.right.pos.short"])
        except:
            self.sleevePatchRightPosShort.SetUndef()

        # update numbers
        #try:
        #    self.numbers.SetStringSelection(kit.attributes["numbers"])
        #except:
        #    self.numbers.SetUndef()

        # update numpal
        #try:
        #    self.numpal.SetStringSelection(kit.attributes["shorts.num-pal.%s" % kit.shortsKey])
        #except:
        #    try: self.numpal.SetStringSelection(kit.attributes["shorts.num-pal"])
        #    except: 
        #        self.numpal.SetUndef()

        # update main color
        try:
            clrValue = kit.attributes.get(
                    "main.color",kit.attributes.get("radar.color"))
            self.radarCS.SetRGBAColour(MakeRGBAColor(clrValue))
        except:
            self.radarCS.ClearColour()

        # update folder
        #try:
        #    self.shirtFolder.SetStringSelection(kit.attributes["shirt.folder"])
        #except:
        #    self.shirtFolder.SetUndef()

        # update folder
        #try:
        #    self.shortsFolder.SetStringSelection(kit.attributes["shorts.folder"])
        #except:
        #    self.shortsFolder.SetUndef()

        # update folder
        #try:
        #    self.socksFolder.SetStringSelection(kit.attributes["socks.folder"])
        #except:
        #    self.socksFolder.SetUndef()

        # mask file
        #try:
        #    self.maskFile.SetStringSelection(kit.attributes["mask"])
        #except:
        #    self.maskFile.SetUndef()

        # mask file
        #try:
        #    self.overlayFile.SetStringSelection(kit.attributes["overlay"])
        #except:
        #    self.overlayFile.SetUndef()

        # update shorts,second,socks colors
        for widget,attr in [
                (self.shortsCS, "shorts.color"),
                (self.secondCS, "second.color"),
                (self.socksCS, "socks.color")]:
            try:
                widget.SetRGBAColour(
                    MakeRGBAColor(kit.attributes[attr]))
            except:
                widget.ClearColour()

        # update icon-type
        try:
            self.iconType.SetStringSelection(kit.attributes["icon.type"])
        except:
            self.iconType.SetUndef()

        # update description
        try:
            self.desc.SetStringSelection(kit.attributes["description"])
        except:
            self.desc.SetUndef()

        # hide True key, show "Check shorts combinations" button
        #for x in range(self.shortsKeys.choice.GetCount()):
        #    self.shortsKeys.choice.Delete(0)
        #self.shortsKeys.Enable(False)
        #if kit != None and kit.teamId != -1:
        #    #self.checkShortsCombos.Enable(True)
        #    # reset shorts-key
        #    kit.shortsKey = os.path.split(kit.foldername)[1]


    def enableControls(self, kit):
        if kit == None or kit.teamId == -1:
            self.collar.Enable(False)
            self.model.Enable(False)
            self.nameShape.Enable(False)
            self.numberSize.Enable(False)
            self.numberY.Enable(False)
            self.frontNumberShow.Enable(False)
            self.frontNumberSize.Enable(False)
            self.frontNumberX.Enable(False)
            self.frontNumberY.Enable(False)
            self.shortsNumLocation.Enable(False)
            self.shortsNumberSize.Enable(False)
            self.shortsNumberX.Enable(False)
            self.shortsNumberY.Enable(False)
            #self.sleevePatch.Enable(False)
            self.sleevePatchLeftPosLong.Enable(False)
            self.sleevePatchLeftPosShort.Enable(False)
            self.sleevePatchRightPosLong.Enable(False)
            self.sleevePatchRightPosShort.Enable(False)
            self.nameShow.Enable(False)
            self.nameSize.Enable(False)
            self.nameY.Enable(False)
            #self.logoLocation.Enable(False)
            #self.numpal.Enable(False)
            #self.numbers.Enable(False)
            #self.shortsKeys.Enable(False)
            #self.checkShortsCombos.Enable(False)
            self.radarCS.Enable(False)
            self.shortsCS.Enable(False)
            self.secondCS.Enable(False)
            self.socksCS.Enable(False)
            self.iconType.Enable(False)
            self.desc.Enable(False)
            #self.shirtFolder.Enable(False)
            #self.shortsFolder.Enable(False)
            #self.socksFolder.Enable(False)
            #self.maskFile.Enable(False)
            #self.overlayFile.Enable(False)
        else:
            self.collar.Enable(True)
            self.model.Enable(True)
            self.nameShape.Enable(True)
            self.numberSize.Enable(True)
            self.numberY.Enable(True)
            self.frontNumberShow.Enable(True)
            self.frontNumberSize.Enable(True)
            self.frontNumberX.Enable(True)
            self.frontNumberY.Enable(True)
            self.shortsNumLocation.Enable(True)
            self.shortsNumberSize.Enable(True)
            self.shortsNumberX.Enable(True)
            self.shortsNumberY.Enable(True)
            #self.sleevePatch.Enable(True)
            self.sleevePatchLeftPosLong.Enable(True)
            self.sleevePatchLeftPosShort.Enable(True)
            self.sleevePatchRightPosLong.Enable(True)
            self.sleevePatchRightPosShort.Enable(True)
            self.nameShow.Enable(True)
            self.nameSize.Enable(True)
            self.nameY.Enable(True)
            #self.logoLocation.Enable(True)
            #self.numpal.Enable(True)
            #self.numbers.Enable(True)
            #self.shortsKeys.Enable(False)
            #self.checkShortsCombos.Enable(True)
            self.radarCS.Enable(True)
            self.shortsCS.Enable(True)
            self.secondCS.Enable(True)
            self.socksCS.Enable(True)
            self.iconType.Enable(True)
            #self.shirtFolder.Enable(True)
            #self.shortsFolder.Enable(True)
            #self.socksFolder.Enable(True)
            #self.maskFile.Enable(True)
            #self.overlayFile.Enable(True)
            self.desc.Enable(True)


    def OnRestore(self, event):
        if self.kitPanel.kit == None:
            return

        self.kitPanel.kit.attribRead = False
        self.selectKit(self.kitPanel.kit)

        # remove kit from list of modified kits
        self.removeKitFromModified()

    """
    Checks if any other shorts for this team have the
    same palette as the shirt from current kit.
    """
    def OnCheckShortsCombos(self, event):
        self.checkShortsCombos.Enable(False)
        teamFolder, kitKey = os.path.split(self.kitPanel.kit.foldername)
        for x in range(self.shortsKeys.choice.GetCount()):
            self.shortsKeys.choice.Delete(0)

        # find all shorts with the same palette
        parent = self.tree.GetItemParent(self.tree.GetSelection())
        allkits = []
        child,cookie = self.tree.GetFirstChild(parent)
        if (child): allkits.append(self.tree.GetItemPyData(child))
        while child:
            child,cookie = self.tree.GetNextChild(parent,cookie)
            if (child): allkits.append(self.tree.GetItemPyData(child))
        kitKeyList = []
        for kit in allkits:
            if hasSamePalette(self.kitPanel.kit.foldername, kit.foldername):
                readAttributes(kit)
                item = os.path.split(kit.foldername)[1]
                self.shortsKeys.choice.Append(item, kit)
                kitKeyList.append(item)

        if len(kitKeyList) > 0:
            # pre-select current kit
            self.shortsKeys.choice.SetSelection(0)
            self.kitPanel.kit.shortsKey = kitKeyList[0]
            for item,i in zip(kitKeyList,range(len(kitKeyList))):
                if item == kitKey:
                    self.shortsKeys.choice.SetSelection(i)
                    self.kitPanel.kit.shortsKey = item
                    break

        self.shortsKeys.Enable(True)
        self.kitPanel.Refresh()

    def addKitToModified(self, kit=None):
        if kit != None:
            self.modified[kit] = True
        else:
            self.modified[self.kitPanel.kit] = True
        # update status bar text
        self.SetStatusText("Modified kits: %d" % len(self.modified.keys()))


    def removeKitFromModified(self):
        try:
            del self.modified[self.kitPanel.kit]
            # update status bar text
            self.SetStatusText("Modified kits: %d" % len(self.modified.keys()))
        except KeyError:
            pass
            

    def OnMenuSave(self, event):
        self.saveChanges()
        self.modified.clear()
        self.SetStatusText("Modified kits: 0");
        print "Changes saved."


    def OnSetKservCfg(self, event):
        print "Linking to kserv.cfg"
        dlg = wx.FileDialog(self, """Navigate to your kserv.cfg file and select it.""",
                defaultDir=os.getcwd(),
                style=wx.DD_DEFAULT_STYLE)
        dlg.SetWildcard("kserv.cfg files|kserv.cfg");

        if dlg.ShowModal() == wx.ID_OK:
            self.kservCfgPath = dlg.GetPath()
            print "You selected %s" % self.kservCfgPath

            # save the value in configuration file
            print "Saving configuration into gdbm.yaml..."
            try:
                cfg = codecs.open(CONFIG_FILE, "wt", "utf-8")
                d = { 'gdbPath':self.gdbPath,
                      'techfitModels':techfitModels,
                      'maxModel':maxModel }
                print d
                cfg.write(yaml.dump(d))
                cfg.close()
            except IOError:
                # unable to save configuration file
                print "Unable to save configuration file"
        else:
            print "Selection cancelled."

        # destroy the dialog after we're done
        dlg.Destroy()

    def OnAbout(self, event):
        dlg = wx.MessageDialog(self, """GDB Manager by Juce.
Version %s from %s

This is a helper program for working with GDB (Graphics Database)
for Kitserver 12. Provides simple visual interface to
define different attributes for kits: 3D-model, collar, locations
for numbers on shirt and shorts, and some others.""" % (VERSION, DATE),
            "About GDB Manager", wx.OK | wx.ICON_INFORMATION)

        dlg.ShowModal()
        dlg.Destroy()

    def OnShowModified(self, event):
        kits = [x.foldername.split('gdb\\')[1] 
                for x in self.modified.keys()]
        kits.sort()
        dlg = wx.SingleChoiceDialog(self, "", 
                "Modified kits: %s" % len(self.modified), 
                kits, wx.CHOICEDLG_STYLE)
        dlg.ShowModal()
        dlg.Destroy()

    def OnResize(self, evt):
        print evt
        self.Layout()

    def OnExit(self, evt):
        print "MyFrame.OnExit"

        # do necessary clean-up and saves
        if self.tree.cancelledOnSaveChanges():
            return

        # Exit the program
        self.Destroy()
        sys.exit(0)


    """
    Saves the changes for altered kits to corresponding config.txt files
    """
    def saveChanges(self, showConfirmation=True):
        print "Saving changes..."

        # TEMP:test
        print "Modified kits [%d]:" % len(self.modified.keys())
        for kit in self.modified.keys():
            # for now: just print out the kit foldername
            print kit.foldername.encode('utf-8')

        # write config.txt for each modified kit
        for kit in self.modified.keys():
            att = None

            # create new file
            try:
                att = codecs.open("%s/%s" % (kit.foldername, "config.txt"), "wt", "utf-8")
            except IOError:
                MessageBox(self, "Unable to save changes", "ERROR: cannot open file %s for writing." % att)
                return

            try:
                # write a comment line, if not already there
                cmt = "# Attribute configuration file auto-generated by GDB Manager %s" % VERSION
                print >>att, cmt
                print >>att
                self.writeSortedAttributes(att, kit)

            except Exception, e:
                MessageBox(self, "Unable to save changes", "ERROR during save: %s" % str(e))

            att.close()

        # show save confirmation message, if asked so.
        if showConfirmation:
            MessageBox(self, "Changes saved", "Your changes were successfully saved.")


    def writeSortedAttributes(self, file, kit):
        keys = kit.attributes.keys()
        keys.sort()
        for name in keys:
            if name=='radar.color': continue # don't write deprecated attr
            if name.startswith("shorts.num-pal.") or name in ["numbers","description","overlay","mask"]:
                print >>file, '%s = "%s"' % (name, kit.attributes[name])
            else:
                print >>file, "%s = %s" % (name, kit.attributes[name])


    """ 
    Returns team name: an ID with helper text, if available.
    """
    def GetTeamText(self, id):
        try:
            return "%s (%s)" % (id, self.teamNames[id])
        except KeyError:
            return "%s" % id


class MyApp(wx.App):
    def OnInit(self):
        frame = MyFrame(None, -1, "GDB Manager")
        frame.Show(1)
        self.SetTopWindow(frame)
        return True


if __name__ == "__main__":
    #app = MyApp(redirect=True, filename="output.log")
    #app = MyApp(redirect=True)
    app = MyApp(0)
    app.MainLoop()

