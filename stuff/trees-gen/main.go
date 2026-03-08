package main

import (
	"fmt"
	"log"
	"os"
	"io/fs"
	"errors"
	"strings"
)
func exists(path string) bool {
  _, err := os.Stat(path)
  if err == nil {
      return true
  }
  if errors.Is(err, fs.ErrNotExist) {
      return false
  }
	return false;
}

func generateTree(outputDir string, baseName string, types []string) {
	output := fmt.Sprint(outputDir + "/" + "output" + ".h");
	if (!exists(outputDir)) {
		err := os.Mkdir(outputDir, os.ModePerm);
		if err != nil {
			log.Fatal(err);
		}
	}
	file, err := os.Create(output);
	if err != nil {
		log.Fatal(err);
	}

	_, err = fmt.Fprintln(file, "template<typename T> struct Visitor;");
	if err != nil {
		log.Fatal(err);
	}
	_, err = fmt.Fprintln(file, "struct " + baseName + " {");
	if err != nil {
		log.Fatal(err);
	}
	_, err = fmt.Fprintln(file, "  template<typename T> T accept(Visitor<T> visitor);");
	if err != nil {
		log.Fatal(err);
	}
	_, err = fmt.Fprintln(file, "};");
	if err != nil {
		log.Fatal(err);
	}

	for _, ttype := range types {
		structName := strings.TrimSpace(strings.Split(ttype, ":")[0]);
		field := strings.TrimSpace(strings.Split(ttype, ":")[1]);
		var errs [100]error;
		_,errs[0] = fmt.Fprintln(file, "\nstruct " + structName + ": public Expr" + " {");
		_,errs[1] = fmt.Fprintln(file, "  " + structName + "(" + field + ") {");
		
		fields := strings.Split(field, ", ");
		for i,member := range fields {
			name := strings.Split(member, " ")[1];
			_,errs[i+3] = fmt.Fprintln(file, "    this->" + name + " = " + name + ";");
		}
		_,errs[2] = fmt.Fprintln(file, "  }");

		for i,member := range fields {
			_,errs[i+3+len(fields)] = fmt.Fprintln(file, "  " + member + ";");
		}
		_, errs[46] = fmt.Fprintln(file, "\n  template<typename T> T accept(Visitor<T> visitor) {");
		_, errs[47] = fmt.Fprintln(file, "    return visitor.visit" + structName + baseName + "(*this);");
		_, errs[48] = fmt.Fprintln(file, "  }");

		_,errs[49] = fmt.Fprintln(file, "};");
		for _,err := range errs {
			if err != nil {
				log.Fatal(err);
			}
		}
	}
	var errs [50]error;
	_, errs[0] = fmt.Fprintln(file, "template<typename T>");
	_, errs[1] = fmt.Fprintln(file, "struct " + "Visitor {");
	for i,ttype := range types {
		name := strings.TrimSpace(strings.Split(ttype, ":")[0]);
		_,errs[i+2] = fmt.Fprintln(file, "  T visit" + name + baseName + "(" +
																name + " " + strings.ToLower(baseName) + ");");
	}
	for _,err := range errs {
		if err != nil {
			log.Fatal(err);
		}
	}
	_, err = fmt.Fprintln(file, "};");
	if err != nil {
		log.Fatal(err);
	}

	err = file.Close();
	if err != nil {
		log.Fatal(err);
	}
}

func main() {
	if (len(os.Args) != 2)	{
		fmt.Println("ERROR: The number of arguments should be equal to one");
		os.Exit(1);
	}
	
	output := os.Args[1];
	types := []string{
		"Binary : Expr* left, Token* _operator, Expr* right",
		"Grouping : Expr* expression",
		"Literal : void* literal",
		"Unary : Token* _operator, Expr* right",
	};
	generateTree(output, "Expr", types);
}
