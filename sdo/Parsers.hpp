

#ifndef _MDL_PARSERS_HPP_
#define _MDL_PARSERS_HPP_

#include <string>
#include "VopFile.hpp"
#include "ExpressionGraph.hpp"
#include "Objective.hpp"

namespace sdo {

/**
 * \brief Parses a vop file.
 * 
 * The path's used to specify model, objective and control files
 * inside the file are made absolute.
 * 
 * \param fileName the vop file given by its filename
 * \return The sdo::VopFile object representing the contents of the vop file
 * \throw std::ifstream::failure if the file cannot be read
 * \throw sdo::parse_error if an error occured while parsing the file
 */
VopFile parse_vop_file(const std::string &fileName);


/**
 * \brief Parses a mdl file.
 * 
 * The mdl file is parsed and the information contained is stored in the given expression
 * graph. Thus it is possible to parse multiple controls into the same expression graph.
 * 
 * It is very important that voc files are parsed before the mdl files or else the controls
 * and changes in the time step are not recognized since a symbol definition will not
 * get overwritten once it is contained in the expression graph.
 * 
 * \param fileName the voc file given by its filename
 * \param exprGraph a reference to the expression graph where the information in the file is stored
 * \throw std::ifstream::failure if the file cannot be read
 * \throw sdo::parse_error if an error occured while parsing the file
 */
void parse_mdl_file(const std::string &fileName, ExpressionGraph &exprGraph);


/**
 * \brief Parses a voc file.
 * 
 * The vocfile is parsed and the information contained is stored in the given expression
 * graph. Thus it is possible to parse multiple controls into the same expression graph.
 * 
 * It is very important that voc files are parsed before the mdl files or else the controls
 * and changes in the time step are not recognized since a symbol definition will not
 * get overwritten once it is contained in the expression graph.
 * 
 * \param fileName the voc file given by its filename
 * \param exprGraph a reference to the sdo::ExpressionGraph where the information in the file is stored
 * \throw std::ifstream::failure if the file cannot be read
 * \throw sdo::parse_error if an error occured while parsing the file
 */
void parse_voc_file(const std::string &fileName, ExpressionGraph &exprGraph);


/**
 * \brief Parse a vpd file.
 * 
 * The vpd file is parsed and the information contained is stored in the given sdo::Objective.
 * Thus it is possible to parse multiple objective files into the same sdo::Objective instance.
 * 
 * \param fileName the voc file given by its filename
 * \param obj a reference to the sdo::Objective where the information in the file is stored
 * \throw std::ifstream::failure if the file cannot be read
 * \throw sdo::parse_error if an error occured while parsing the file
 */
void parse_vpd_file(const std::string &fileName, Objective& obj);

}


#endif