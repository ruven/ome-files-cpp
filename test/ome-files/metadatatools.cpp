/*
 * #%L
 * OME-FILES C++ library for image IO.
 * %%
 * Copyright © 2006 - 2015 Open Microscopy Environment:
 *   - Massachusetts Institute of Technology
 *   - National Institutes of Health
 *   - University of Dundee
 *   - Board of Regents of the University of Wisconsin-Madison
 *   - Glencoe Software, Inc.
 * %%
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of any organization.
 * #L%
 */

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/range/size.hpp>

#include <ome/files/FormatException.h>
#include <ome/files/MetadataTools.h>

#include <ome/test/test.h>
#include <ome/test/io.h>

#include <ome/common/module.h>
#include <ome/files/module.h>

#include <ome/common/xml/Platform.h>
#include <ome/common/xml/dom/Document.h>

#include <ome/xml/Document.h>
#include <ome/xml/version.h>
#include <ome/xml/OMETransformResolver.h>
#include <ome/xml/OMETransformResolver.h>
#include <ome/xml/model/enums/EnumerationException.h>

using boost::filesystem::path;
using boost::filesystem::directory_iterator;
using ome::files::dimension_size_type;
using ome::files::createID;
using ome::files::createDimensionOrder;
using ome::files::createOMEXMLMetadata;
using ome::files::validateModel;
using ome::files::FormatException;
using ome::xml::model::enums::DimensionOrder;
using namespace ome::xml::model::primitives;

TEST(MetadataToolsTest, CreateID1)
{
  std::string e1(createID("Instrument", 0));
  ASSERT_EQ(std::string("Instrument:0"), e1);

  std::string e2(createID("Instrument", 2));
  ASSERT_EQ(std::string("Instrument:2"), e2);

  std::string i1(createID("Image", 4));
  ASSERT_EQ(std::string("Image:4"), i1);
}

TEST(MetadataToolsTest, CreateID2)
{
  std::string d1(createID("Detector", 0, 0));
  ASSERT_EQ(std::string("Detector:0:0"), d1);

  std::string d2(createID("Detector", 2, 5));
  ASSERT_EQ(std::string("Detector:2:5"), d2);

  std::string i1(createID("Shape", 4, 3));
  ASSERT_EQ(std::string("Shape:4:3"), i1);
}

TEST(MetadataToolsTest, CreateID3)
{
  std::string m1(createID("Mask", 0, 0, 0));
  ASSERT_EQ(std::string("Mask:0:0:0"), m1);

  std::string m2(createID("Mask", 3, 5, 6));
  ASSERT_EQ(std::string("Mask:3:5:6"), m2);

  std::string m3(createID("Mask", 92, 329, 892));
  ASSERT_EQ(std::string("Mask:92:329:892"), m3);
}

TEST(MetadataToolsTest, CreateID4)
{
  std::string u1(createID("Unknown", 0, 0, 0, 0));
  ASSERT_EQ(std::string("Unknown:0:0:0:0"), u1);

  std::string u2(createID("Unknown", 5, 23, 6, 3));
  ASSERT_EQ(std::string("Unknown:5:23:6:3"), u2);

  std::string u3(createID("Unknown", 9, 2, 4, 2));
  ASSERT_EQ(std::string("Unknown:9:2:4:2"), u3);
}

TEST(MetadataToolsTest, CurrentModelVersion)
{
  ASSERT_EQ(std::string(OME_XML_MODEL_VERSION), ome::files::getModelVersion());
}

TEST(MetadataToolsTest, ModelVersionFromString)
{
  std::string xml;
  boost::filesystem::path sample_path(ome::common::module_runtime_path("ome-xml-sample"));

  readFile(sample_path / "2012-06/multi-channel-z-series-time-series.ome.xml", xml);
  ASSERT_EQ(std::string("2012-06"), ome::files::getModelVersion(xml));
}

TEST(MetadataToolsTest, ModelVersionFromDocument)
{
  ome::common::xml::Platform xmlplat;

  std::string xml;
  boost::filesystem::path sample_path(ome::common::module_runtime_path("ome-xml-sample"));

  readFile(sample_path / "2013-06/multi-channel-z-series-time-series.ome.xml", xml);

  ome::common::xml::dom::Document doc = ome::xml::createDocument(xml);
  ASSERT_TRUE(doc);

  ASSERT_EQ(std::string("2013-06"), ome::files::getModelVersion(doc));
}

TEST(MetadataToolsTest, CreateDimensionOrder)
{
  EXPECT_EQ(DimensionOrder::XYZTC, createDimensionOrder(""));
  EXPECT_EQ(DimensionOrder::XYZTC, createDimensionOrder("XYXYZTCZ"));
  EXPECT_EQ(DimensionOrder::XYCZT, createDimensionOrder("XYC"));
  EXPECT_EQ(DimensionOrder::XYTZC, createDimensionOrder("XYTZ"));

  EXPECT_THROW(createDimensionOrder("CXY"), ome::xml::model::enums::EnumerationException);
  EXPECT_THROW(createDimensionOrder("Y"), ome::xml::model::enums::EnumerationException);
  EXPECT_THROW(createDimensionOrder("YC"), ome::xml::model::enums::EnumerationException);
}

struct ModelState
{
  dimension_size_type sizeC;
  dimension_size_type channelCount;
  dimension_size_type samples[6];
};

struct Corrections
{
  path filename;
  bool initiallyValid;
  bool correctable;
  dimension_size_type imageIndex;
  ModelState before;
  ModelState after;
};

typedef ModelState MS;
typedef Corrections Corr;

const std::vector<Corrections> corrections
  {
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/validchannels.ome"),
      true,
      true,
      0,
      { 1, 1, { 1, 0, 0, 0, 0, 0 } },
      { 1, 1, { 1, 0, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/validchannels.ome"),
      true,
      true,
      1,
      { 4, 4, { 1, 1, 1, 1, 0, 0 } },
      { 4, 4, { 1, 1, 1, 1, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/validchannels.ome"),
      true,
      true,
      2,
      { 3, 1, { 3, 0, 0, 0, 0, 0 } },
      { 3, 1, { 3, 0, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/validchannels.ome"),
      true,
      true,
      3,
      { 6, 2, { 3, 0, 0, 0, 0, 0 } },
      { 6, 2, { 3, 0, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/validchannels.ome"),
      true,
      true,
      4,
      { 4, 2, { 1, 3, 0, 0, 0, 0 } },
      { 4, 2, { 1, 3, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      0,
      { 1, 1, { 1, 0, 0, 0, 0, 0 } },
      { 1, 1, { 1, 0, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      1,
      { 1, 0, { 0, 0, 0, 0, 0, 0 } },
      { 1, 1, { 1, 0, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      2,
      { 1, 1, { 2, 0, 0, 0, 0, 0 } },
      { 2, 1, { 2, 0, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      3,
      { 4, 1, { 1, 0, 0, 0, 0, 0 } },
      { 1, 1, { 1, 0, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      4,
      { 1, 1, { 0, 0, 0, 0, 0, 0 } },
      { 1, 1, { 1, 0, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      5,
      { 4, 4, { 1, 1, 1, 1, 0, 0 } },
      { 4, 4, { 1, 1, 1, 1, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      6,
      { 4, 3, { 1, 1, 1, 0, 0, 0 } },
      { 3, 3, { 1, 1, 1, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      7,
      { 4, 0, { 0, 0, 0, 0, 0, 0 } },
      { 4, 4, { 1, 1, 1, 1, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      8,
      { 4, 4, { 2, 2, 1, 1, 0, 0 } },
      { 6, 4, { 2, 2, 1, 1, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      9,
      { 7, 4, { 1, 1, 1, 1, 0, 0 } },
      { 4, 4, { 1, 1, 1, 1, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      10,
      { 4, 4, { 0, 1, 0, 1, 0, 0 } },
      { 4, 4, { 1, 1, 1, 1, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      11,
      { 4, 4, { 0, 0, 0, 0, 0, 0 } },
      { 4, 4, { 1, 1, 1, 1, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      12,
      { 3, 1, { 3, 0, 0, 0, 0, 0 } },
      { 3, 1, { 3, 0, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      13,
      { 3, 0, { 0, 0, 0, 0, 0, 0 } },
      { 3, 3, { 1, 1, 1, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      14,
      { 3, 1, { 5, 0, 0, 0, 0, 0 } },
      { 5, 1, { 5, 0, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      15,
      { 2, 1, { 3, 0, 0, 0, 0, 0 } },
      { 3, 1, { 3, 0, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      16,
      { 3, 1, { 0, 0, 0, 0, 0, 0 } },
      { 3, 1, { 3, 0, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      17,
      { 6, 2, { 3, 3, 0, 0, 0, 0 } },
      { 6, 2, { 3, 3, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      18,
      { 6, 1, { 3, 0, 0, 0, 0, 0 } },
      { 3, 1, { 3, 0, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      19,
      { 6, 0, { 0, 0, 0, 0, 0, 0 } },
      { 6, 6, { 1, 1, 1, 1, 1, 1 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      20,
      { 6, 2, { 5, 3, 0, 0, 0, 0 } },
      { 8, 2, { 5, 3, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      21,
      { 9, 2, { 3, 3, 0, 0, 0, 0 } },
      { 6, 2, { 3, 3, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      22,
      { 6, 2, { 3, 0, 0, 0, 0, 0 } },
      { 6, 2, { 3, 3, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      23,
      { 6, 2, { 0, 0, 0, 0, 0, 0 } },
      { 6, 2, { 3, 3, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      24,
      { 4, 2, { 1, 3, 0, 0, 0, 0 } },
      { 4, 2, { 1, 3, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      25,
      { 4, 1, { 1, 0, 0, 0, 0, 0 } },
      { 1, 1, { 1, 0, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      26,
      { 4, 0, { 0, 0, 0, 0, 0, 0 } },
      { 4, 4, { 1, 1, 1, 1, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      27,
      { 4, 2, { 5, 3, 0, 0, 0, 0 } },
      { 8, 2, { 5, 3, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      28,
      { 2, 2, { 1, 3, 0, 0, 0, 0 } },
      { 4, 2, { 1, 3, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      29,
      { 4, 2, { 1, 0, 0, 0, 0, 0 } },
      { 4, 2, { 1, 3, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      30,
      { 4, 2, { 0, 3, 0, 0, 0, 0 } },
      { 4, 2, { 1, 3, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-correctable.ome"),
      false,
      true,
      31,
      { 4, 2, { 0, 0, 0, 0, 0, 0 } },
      { 4, 2, { 2, 2, 0, 0, 0, 0 } }
    },
    {
      path(PROJECT_SOURCE_DIR "/test/ome-files/data/brokenchannels-uncorrectable.ome"),
      false,
      false,
      0,
      { 4, 3, { 1, 0, 0, 0, 0, 0 } },
      { 4, 3, { 1, 0, 0, 0, 0, 0 } }
    }
  };

template<class charT, class traits>
inline std::basic_ostream<charT,traits>&
operator<< (std::basic_ostream<charT,traits>& os,
            const Corrections& params)
{
  return os << params.filename << ": Image #" << params.imageIndex;
}

class CorrectionTest : public ::testing::TestWithParam<Corrections>
{
};

TEST_P(CorrectionTest, ValidateAndCorrectModel)
{
  ome::common::xml::Platform xmlplat;

  const Corrections& current(GetParam());
  const dimension_size_type idx(current.imageIndex);

  ome::common::xml::dom::Document doc = ome::xml::createDocument(current.filename);
  ASSERT_TRUE(doc);

  ASSERT_EQ(std::string("2013-06"), ome::files::getModelVersion(doc));

  std::shared_ptr<::ome::xml::meta::OMEXMLMetadata> meta(createOMEXMLMetadata(doc));

  {
    const ModelState& state(current.before);

    EXPECT_EQ(PositiveInteger(state.sizeC), meta->getPixelsSizeC(idx));
    EXPECT_EQ(state.channelCount, meta->getChannelCount(idx));
    for (dimension_size_type s = 0; s < boost::size(state.samples); ++s)
      {
        if (state.samples[s] > 0)
          {
            EXPECT_EQ(PositiveInteger(state.samples[s]),
                      meta->getChannelSamplesPerPixel(idx, s));
          }
      }
  }

  if (current.initiallyValid)
    {
      EXPECT_TRUE(validateModel(*meta, false));
    }
  else
    {
      EXPECT_FALSE(validateModel(*meta, false));
      if (current.correctable)
        {
          EXPECT_NO_THROW(EXPECT_FALSE(validateModel(*meta, true)));
        }
      else
        {
          // Totally broken; end test here.
          EXPECT_THROW(validateModel(*meta, true), FormatException);
          return;
        }
    }
  // Model should now be valid.
  EXPECT_TRUE(validateModel(*meta, false));

  {
    const ModelState& state(current.after);

    EXPECT_EQ(PositiveInteger(state.sizeC), meta->getPixelsSizeC(idx));
    EXPECT_EQ(state.channelCount, meta->getChannelCount(idx));
    for (dimension_size_type s = 0; s < boost::size(state.samples); ++s)
      {
        if (state.samples[s] > 0)
          {
            EXPECT_EQ(PositiveInteger(state.samples[s]),
                      meta->getChannelSamplesPerPixel(idx, s));
          }
      }
  }
}

struct ModelTestParameters
{
  path file;
};

template<class charT, class traits>
inline std::basic_ostream<charT,traits>&
operator<< (std::basic_ostream<charT,traits>& os,
            const ModelTestParameters& p)
{
  return os << p.file;
}

namespace
{

  std::vector<ModelTestParameters>
  find_model_tests()
  {
    std::vector<ModelTestParameters> params;

    ome::xml::OMETransformResolver tr;
    std::set<std::string> versions = tr.schema_versions();

    ome::files::register_module_paths();
    boost::filesystem::path sample_path(ome::common::module_runtime_path("ome-xml-sample"));
    if (exists(sample_path) && is_directory(sample_path))
      {
        for (directory_iterator si(sample_path); si != directory_iterator(); ++si)
          {
            if (versions.find(si->path().filename().string()) == versions.end())
              continue; // Not a schema directory with transforms.
            path schemadir(si->path());
            if (exists(schemadir) && is_directory(schemadir))
              {
                for (directory_iterator fi(schemadir); fi != directory_iterator(); ++fi)
                  {
                    ModelTestParameters p;
                    p.file = *fi;

                    // 2008-09/instrument.ome.xml
                    if (schemadir.filename() == path("2008-09") &&
                        p.file.filename() == path("instrument.ome.xml"))
                      continue;
                    // timestampannotation.ome.xml - Contains non-POSIX timestamps.
                    if (p.file.filename() == path("timestampannotation.ome.xml"))
                      continue;
                    // Map Annotation cannot be converted
                    if (p.file.filename() == path("mapannotation.ome.xml"))
                      continue;

                    if (p.file.extension() == path(".ome") ||
                        p.file.extension() == path(".xml"))
                      params.push_back(p);
                  }
              }
          }
      }

    return params;
  }

}

std::vector<ModelTestParameters> model_params(find_model_tests());

class ModelTest : public ::testing::TestWithParam<ModelTestParameters>
{
public:
  void SetUp()
  {
    const ModelTestParameters& params = GetParam();
    std::cout << "Source file " << params.file << '\n';
  }
};

TEST_P(ModelTest, CreateMetadataFromFile)
{
  const ModelTestParameters& params = GetParam();

  std::shared_ptr<::ome::xml::meta::OMEXMLMetadata> meta;
  ASSERT_NO_THROW(meta = createOMEXMLMetadata(params.file));
}

TEST_P(ModelTest, CreateMetadataFromStream)
{
  const ModelTestParameters& params = GetParam();

  boost::filesystem::ifstream input(params.file);

  std::shared_ptr<::ome::xml::meta::OMEXMLMetadata> meta;
  ASSERT_NO_THROW(meta = createOMEXMLMetadata(input));
}

TEST_P(ModelTest, CreateMetadataFromString)
{
  const ModelTestParameters& params = GetParam();

  std::string input;
  readFile(params.file, input);

  std::shared_ptr<::ome::xml::meta::OMEXMLMetadata> meta;
  ASSERT_NO_THROW(meta = createOMEXMLMetadata(input));
}

// Disable missing-prototypes warning for INSTANTIATE_TEST_CASE_P;
// this is solely to work around a missing prototype in gtest.
#ifdef __GNUC__
#  if defined __clang__ || defined __APPLE__
#    pragma GCC diagnostic ignored "-Wmissing-prototypes"
#  endif
#  pragma GCC diagnostic ignored "-Wmissing-declarations"
#endif

INSTANTIATE_TEST_CASE_P(CorrectionVariants, CorrectionTest, ::testing::ValuesIn(corrections));
INSTANTIATE_TEST_CASE_P(ModelVariants, ModelTest, ::testing::ValuesIn(model_params));
