var fso = new ActiveXObject("Scripting.FileSystemObject");
var folder = fso.GetFolder("Shader");
var scriptPath = WScript.ScriptFullName.substr(0, WScript.ScriptFullName.lastIndexOf("\\") + 1);
var vsFileList = new Array;
var psFileList = new Array;

var files= new Enumerator(folder.Files);
for( files.moveFirst(); !files.atEnd(); files.moveNext() )
{
	// ファイル名のみ取り出し（拡張子を除く
	var fileName = files.item().Name;
	fileName = fileName.substr(0, fileName.indexOf("."));

	// ファイル名のシェーダータイプに応じて振り分け
	if(fileName.indexOf("PS") != -1)
	{
		psFileList.push(fileName);
	}
	else if(fileName.indexOf("VS") != -1)
	{
		vsFileList.push(fileName);
	}
	else if (fileName.indexOf(".h") != -1 && fileName.indexOf(".cpp") != -1)
	{
		WScript.echo(fileName + ": VS? PS?");
	}
}

// .cpp置き換え
var sourceFile = fso.OpenTextFile(scriptPath + "_ShaderList.cpp", 1);
var sourceFileText = sourceFile.ReadAll();
sourceFile.Close();
var vsFiles = "";
for(var file in vsFileList)
{
	vsFiles += "\t\"Assets/" + vsFileList[file] + ".cso\",\r\n";
}
sourceFileText = sourceFileText.replace("__VS_FILE__", vsFiles);
var psFiles = "";
for(var file in psFileList)
{
	psFiles += "\t\"Assets/" + psFileList[file] + ".cso\",\r\n";
}
sourceFileText = sourceFileText.replace("__PS_FILE__", psFiles);
sourceFile = fso.OpenTextFile(scriptPath + "ShaderList.cpp", 2, true);
sourceFile.Write(sourceFileText);
sourceFile.Close();

// .h置き換え
var headerFile = fso.OpenTextFile(scriptPath + "_ShaderList.h", 1);
var headerFileText = headerFile.ReadAll();
headerFile.Close();
var reg = /[A-Z]+[a-z]*/g;
var vsKinds = "";
for(var kind in vsFileList)
{
	var matchs = vsFileList[kind].match(reg);
	var str = matchs[0].toUpperCase();
	for(var i = 1; i < matchs.length; ++i)
	{
		str += "_" + matchs[i].toUpperCase();
	}
	vsKinds += "\t" + str + ",\r\n";
}
headerFileText = headerFileText.replace("__VS_KIND__", vsKinds);
var psKinds = "";
for(var kind in psFileList)
{
	var matchs = psFileList[kind].match(reg);
	var str = matchs[0].toUpperCase();
	for(var i = 1; i < matchs.length; ++i)
	{
		str += "_" + matchs[i].toUpperCase();
	}
	psKinds += "\t" + str + ",\r\n";
}
headerFileText = headerFileText.replace("__PS_KIND__", psKinds);
headerFile = fso.OpenTextFile(scriptPath + "ShaderList.h", 2, true);
headerFile.Write(headerFileText);
headerFile.Close();

// ファイル移動
fso.CopyFile(scriptPath + "ShaderList.cpp", "Shader/", true);
fso.CopyFile(scriptPath + "ShaderList.h", "Shader/", true);