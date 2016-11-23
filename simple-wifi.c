/*
 * simple-driver.c -- Simple device driver to test some WIFI capabalities
 * Includes:
 *              - SCAN
 *              - ASSOCIATE
 *              - Tx
 *
 * Author: Prem Mallappa <prem.mallappa@gmail.com>
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>

#include <linux/sched.h>
#include <linux/kernel.h>                       /* printk() */
#include <linux/slab.h>                         /* kmalloc() */
#include <linux/errno.h>                        /* error codes */
#include <linux/types.h>                        /* size_t */
#include <linux/interrupt.h>                    /* mark_bh */

#if 0
#include <linux/in.h>
#include <linux/netdevice.h>              /* struct device, and other headers */
#include <linux/etherdevice.h>            /* eth_type_trans */
#include <linux/ip.h>                     /* struct iphdr */
#include <linux/tcp.h>                    /* struct tcphdr */
#include <linux/skbuff.h>

#include <linux/in6.h>
#include <asm/checksum.h>
#endif


#include <linux/nl80211.h>
#include <net/mac80211.h>

MODULE_AUTHOR("Prem Mallappa");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Dummy device driver to demonstrate SCAN, ASSOCIATE and Tx capabilities\n");

#define ENTRY() pr_info("Entered %s\n", __func__)
#define EXIT() pr_info("Exit %s\n", __func__)

static int simple_wifi_start(struct ieee80211_hw *hw)
{
	struct simple_wifi *sw = hw->priv;
	ENTRY();
	if (!sw)
		return -1;

	EXIT();
	return 0;
}


static void simple_wifi_stop(struct ieee80211_hw *hw)
{
	struct simple_wifi *sw = hw->priv;

	ENTRY();
	if (!sw)
		printk(KERN_CRIT "Something Wrong\n");

	EXIT();
}

void simple_wifi_tx(struct ieee80211_hw *hw,
		    struct ieee80211_tx_control *control,
		    struct sk_buff *skb)
{
	ENTRY();
	EXIT();
}

int simple_wifi_add_interface(struct ieee80211_hw *hw,
			      struct ieee80211_vif *vif)
{
	ENTRY();
	EXIT();
	return 0;
}

void simple_wifi_remove_interface(struct ieee80211_hw *hw,
				   struct ieee80211_vif *vif)
{
	ENTRY();
	EXIT();
}

void simple_wifi_configure_filter(struct ieee80211_hw *hw,
				  unsigned int changed_flags,
				  unsigned int *total_flags,
				  u64 multicast)
{
	ENTRY();
	EXIT();
}

static struct ieee80211_ops simple_wifi_ops = {
	.start = simple_wifi_start,
	.stop = simple_wifi_stop,
	.tx = simple_wifi_tx,
	.add_interface = simple_wifi_add_interface,
	.remove_interface = simple_wifi_remove_interface,
	.configure_filter = simple_wifi_configure_filter,
#if 0
	.change_interface = simple_wifi_change_interface,
	.config = simple_wifi_config,
	.set_key = simple_wifi_set_key,
	.conf_tx = simple_wifi_conf_tx,
	.bss_info_changed = simple_wifi_bss_info_changed,
	.get_tsf = simple_wifi_get_tsf,
	.set_tsf = simple_wifi_set_tsf,
	.reset_tsf = simple_wifi_reset_tsf,
	.sta_notify = simple_wifi_sta_notify,
	.ampdu_action = simple_wifi_ampdu_action,
	.sw_scan_start = simple_wifi_sw_scan_start,
	.sw_scan_complete = simple_wifi_sw_scan_complete,
	.rfkill_poll = simple_wifi_rfkill_poll,
	.sta_add = simple_wifi_sta_add,
	.sta_remove = simple_wifi_sta_remove,
	.flush = simple_wifi_flush,
#endif
};

#define SIMPLE_CHAN_MAX 12
#define SIMPLE_RATE_MAX 12

struct simple_wifi {
	struct ieee80211_hw *hw; /* mac80211 instance */

	struct ieee80211_supported_band
	bands[NUM_NL80211_BANDS];
};

struct simple_wifi_rates {
	unsigned short bitrate; /* In 100kbit/s */
	unsigned short ratemask;
} simple_wifi_rates[] = {
	{
		.bitrate = 10,
		.ratemask = BIT(0),
	},
	{
		.bitrate = 20,
		.ratemask = BIT(1),
	},
	{
		.bitrate = 55,
		.ratemask = BIT(2),
	},
	{
		.bitrate = 110,
		.ratemask = BIT(3),
	},
	{
		.bitrate = 60,
		.ratemask = BIT(4),
	}
};

static void simple_wifi_rate(struct ieee80211_rate *entry,
			     const u16 index, unsigned short bitrate)
{
	entry->flags = 0;
	entry->bitrate = bitrate;
	entry->hw_value = index;
	entry->hw_value_short = index;
	/* Hmmm ? */
	entry->flags |= IEEE80211_RATE_SHORT_PREAMBLE;
}

static void simple_wifi_channel(struct ieee80211_channel *entry,
				const int channel, const int tx_power,
				const int value)
{
	entry->band = channel <= 14 ? NL80211_BAND_2GHZ : NL80211_BAND_5GHZ;
	entry->center_freq = ieee80211_channel_to_frequency(channel,
							    entry->band);
	entry->hw_value = value;
	entry->max_power = tx_power;
	entry->max_antenna_gain = 0xff;
}

static int __init simple_wifi_attach(struct simple_wifi *sw)
{
	struct ieee80211_hw *hw = sw->hw;
	u8 macaddr[ETH_ALEN] = {00, 11, 22, 33, 44, 55};
	struct ieee80211_channel *channels;
	struct ieee80211_rate *rates;
	int error, i;
	int num_rates = ARRAY_SIZE(simple_wifi_rates);
	const int num_channels = 4;

	ieee80211_hw_set(hw, HAS_RATE_CONTROL);
	ieee80211_hw_set(hw, RX_INCLUDES_FCS);
	ieee80211_hw_set(hw, SIGNAL_DBM);

	SET_IEEE80211_PERM_ADDR(hw, macaddr);

	/* setup channels and rates */
	channels = kcalloc(num_channels, sizeof(*channels), GFP_KERNEL);
	if (!channels)
		return -ENOMEM;

	rates = kcalloc(num_rates, sizeof(*rates), GFP_KERNEL);
	if (!rates)
		goto exit_free_channels;

	for (i = 0; i < num_rates; i++)
		simple_wifi_rate(&rates[i], i, simple_wifi_rates[i].bitrate);

	for (i = 0; i < num_channels; i++) {
                simple_wifi_channel(&channels[i], i,
				    0xf, /* max_tx_power */
				    i);
	}

	/* Channels: 2.4 GHz  */
	{
		sw->bands[NL80211_BAND_2GHZ].n_channels = num_channels;
		sw->bands[NL80211_BAND_2GHZ].n_bitrates = num_rates;
		sw->bands[NL80211_BAND_2GHZ].channels = channels;
		sw->bands[NL80211_BAND_2GHZ].bitrates = rates;
		hw->wiphy->bands[NL80211_BAND_2GHZ] =
			&sw->bands[NL80211_BAND_2GHZ];
	}

	hw->wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION);
	hw->queues = 1;

	error = ieee80211_register_hw(hw);
	if (error) {
		goto bad;
	}

	return 0;

exit_free_channels:
	kfree(channels);
bad:
	return -1;
}

/*
 * Since we dont have a proper mechanism to get the dev interface,
 * We should allocate one and release it properly
 */
static struct device *dev = NULL;

static int __init init_simple_wifi(void)
{
	struct ieee80211_hw *hw;
	struct simple_wifi *sw;
	int ret = 0;

	dev = kzalloc(sizeof(struct device), GFP_KERNEL);
	if (!dev) {
		goto bad;
	}

	//hw = ieee80211_alloc_hw(sizeof(struct simple_wifi), &simple_wifi_ops);
	hw = ieee80211_alloc_hw_nm(sizeof(*sw), &simple_wifi_ops, "swifi");
	if (!hw) {
		printk(KERN_ERR "simple_wifi: no memory for ieee80211_hw\n");
		goto free_dev;
	}

	SET_IEEE80211_DEV(hw, dev);

	sw = hw->priv;
	sw->hw = hw;
	dev_set_drvdata(dev, sw);

	ret = simple_wifi_attach(sw);
	if (ret)
		goto free_80211;

	return 0;

free_80211:
	ieee80211_free_hw(hw);
free_dev:
	kfree(dev);
bad:
	return ret;
}
module_init(init_simple_wifi);

static void __exit exit_simple_wifi(void)
{
	if (dev) {
		struct simple_wifi *sw = dev_get_drvdata(dev);
		struct ieee80211_hw *hw = sw->hw;

		ieee80211_free_hw(hw);
		kfree(sw);
		kfree(dev);
	}
}
module_exit(exit_simple_wifi);
