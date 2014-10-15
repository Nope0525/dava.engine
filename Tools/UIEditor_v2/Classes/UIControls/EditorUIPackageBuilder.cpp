//
//  EditorUIPackageBuilder.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 11.10.14.
//
//

#include "EditorUIPackageBuilder.h"

#include "PackageHierarchy/PackageNode.h"
#include "PackageHierarchy/ImportedPackagesNode.h"
#include "PackageHierarchy/PackageControlsNode.h"
#include "UIControls/ControlProperties/ControlPropertiesSection.h"
#include "UIControls/ControlProperties/BackgroundPropertiesSection.h"
#include "UIControls/ControlProperties/InternalControlPropertiesSection.h"
#include "UIControls/ControlProperties/ValueProperty.h"
#include "UIControls/ControlProperties/LocalizedTextValueProperty.h"
#include "UIControls/PackageHierarchy/ControlNode.h"

using namespace DAVA;

EditorUIPackageBuilder::EditorUIPackageBuilder() : packageNode(NULL), currentObject(NULL)
{
    
}

EditorUIPackageBuilder::~EditorUIPackageBuilder()
{
    
}

UIPackage *EditorUIPackageBuilder::BeginPackage(const FilePath &packagePath)
{
    DVASSERT(packageNode == NULL);
    UIPackage *package = new UIPackage(packagePath);
    packageNode = new PackageNode(package);
    return package;
}

void EditorUIPackageBuilder::EndPackage()
{
    DVASSERT(packageNode != NULL);
}

UIPackage * EditorUIPackageBuilder::ProcessImportedPackage(const String &packagePath, AbstractUIPackageLoader *loader)
{
    // store state
    PackageNode *prevPackageNode = packageNode;
    DAVA::Vector<ControlNode*> prevControlsStack = controlsStack;
    DAVA::BaseObject *prevObj = currentObject;
    PropertiesSection *prevSect = currentSection;

    // clear state
    packageNode = NULL;
    controlsStack.clear();
    currentObject = NULL;
    currentSection = NULL;
    
    // load package
    UIPackage *result = loader->LoadPackage(packagePath);
    PackageControlsNode *controlsNode = SafeRetain(packageNode->GetPackageControlsNode());
    controlsNode->SetName(packageNode->GetName());
    SafeRelease(packageNode);
    
    prevPackageNode->GetImportedPackagesNode()->Add(controlsNode);
    SafeRelease(controlsNode);

    // restore state
    packageNode = prevPackageNode;
    controlsStack = prevControlsStack;
    currentObject = prevObj;
    currentSection = prevSect;

    return result;
}

UIControl *EditorUIPackageBuilder::BeginControlWithClass(const String className)
{
    UIControl *control = ObjectFactory::Instance()->New<UIControl>(className);
    ControlNode *node = new ControlNode(control);
    AddControlNode(node);
    return control;
}

UIControl *EditorUIPackageBuilder::BeginControlWithCustomClass(const String customClassName, const String className)
{
    UIControl *control = ObjectFactory::Instance()->New<UIControl>(className);
    control->SetCustomControlClassName(customClassName);
    ControlNode *node = new ControlNode(control);
    AddControlNode(node);
    return control;
}

UIControl *EditorUIPackageBuilder::BeginControlWithPrototype(const String &packageName, const String &prototypeName, const String &customClassName, AbstractUIPackageLoader *loader)
{
    ControlNode *prototypeNode = NULL;
    UIPackage *prototypePackage = NULL;
    if (packageName.empty())
    {
        prototypeNode = packageNode->GetPackageControlsNode()->FindControlNodeByName(prototypeName);
        if (!prototypeNode)
        {
            if (loader->LoadControlByName(prototypeName))
                prototypeNode = packageNode->GetPackageControlsNode()->FindControlNodeByName(prototypeName);
        }
    }
    else
    {
        PackageControlsNode *importedPackage = packageNode->GetImportedPackagesNode()->FindPackageControlsNodeByName(packageName);
        if (importedPackage)
        {
            prototypePackage = importedPackage->GetPackage();
            prototypeNode = importedPackage->FindControlNodeByName(prototypeName);
        }
    }
    DVASSERT(prototypeNode);
    ControlNode *node = new ControlNode(prototypeNode, prototypePackage);
    node->GetControl()->SetCustomControlClassName(customClassName);
    AddControlNode(node);
    return node->GetControl();
}

UIControl *EditorUIPackageBuilder::BeginControlWithPath(const String &pathName)
{
    ControlNode *control = NULL;
    if (!controlsStack.empty())
    {
        control = controlsStack.back();
        Vector<String> controlNames;
        Split(pathName, "/", controlNames, false, true);
        for (Vector<String>::const_iterator iter = controlNames.begin(); iter!=controlNames.end(); ++iter)
        {
            control = control->FindByName(*iter);
            if (!control)
                break;
        }
    }
    
    DVASSERT(control != NULL);
    controlsStack.push_back(SafeRetain(control));
    return control->GetControl();
}

UIControl *EditorUIPackageBuilder::BeginUnknownControl(const YamlNode *node)
{
    DVASSERT(false);
    return NULL;
}

void EditorUIPackageBuilder::EndControl()
{
    ControlNode *lastControl = controlsStack.back();
    controlsStack.pop_back();
    if (lastControl)
        SafeRelease(lastControl);
}

void EditorUIPackageBuilder::BeginControlPropretiesSection(const String &name)
{
    currentSection = controlsStack.back()->GetPropertiesRoot()->GetControlPropertiesSection(name);
    currentObject = controlsStack.back()->GetControl();
}

void EditorUIPackageBuilder::EndControlPropertiesSection()
{
    currentSection = NULL;
    currentObject = NULL;
}

UIControlBackground *EditorUIPackageBuilder::BeginBgPropertiesSection(int index, bool sectionHasProperties)
{
    ControlNode *node = controlsStack.back();
    BackgroundPropertiesSection *section = node->GetPropertiesRoot()->GetBackgroundPropertiesSection(index);
    if (section && sectionHasProperties)
    {
        if (section->GetBg() == NULL)
            section->CreateControlBackground();

        if (section->GetBg())
        {
            currentObject = section->GetBg();
            currentSection = section;
            return section->GetBg();
        }
    }
    
    return NULL;
}

void EditorUIPackageBuilder::EndBgPropertiesSection()
{
    currentSection = NULL;
    currentObject = NULL;
}

UIControl *EditorUIPackageBuilder::BeginInternalControlSection(int index, bool sectionHasProperties)
{
    ControlNode *node = controlsStack.back();
    InternalControlPropertiesSection *section = node->GetPropertiesRoot()->GetInternalControlPropertiesSection(index);
    if (section && sectionHasProperties)
    {
        if (section->GetInternalControl() == NULL)
            section->CreateInternalControl();
        
        if (section->GetInternalControl())
        {
            currentObject = section->GetInternalControl();
            currentSection = section;
            return section->GetInternalControl();
        }
    }
    
    return NULL;
}

void EditorUIPackageBuilder::EndInternalControlSection()
{
    currentSection = NULL;
    currentObject = NULL;
}

void EditorUIPackageBuilder::ProcessProperty(const InspMember *member, const VariantType &value)
{
    if (currentObject && currentSection && (member->Flags() & I_EDIT))
    {
        ValueProperty *property = currentSection->FindProperty(member);
        if (property && value.GetType() != VariantType::TYPE_NONE)
            property->SetValue(value);
    }
}

void EditorUIPackageBuilder::AddControlNode(ControlNode *node)
{
    if (controlsStack.empty())
        packageNode->GetPackageControlsNode()->Add(node);
    else
        controlsStack.back()->Add(node);

    controlsStack.push_back(node);
    
}
